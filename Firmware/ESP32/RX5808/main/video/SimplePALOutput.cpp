#include "SimplePALOutput.h"
#include "esp_idf_version.h"
#include "esp_heap_caps.h"
#include "esp_attr.h"
#include "esp_intr_alloc.h"
#include "esp_err.h"
#include "soc/gpio_reg.h"
#include "soc/rtc.h"
#include "soc/soc.h"
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/io_mux_reg.h"
#include "esp32/rom/gpio.h"
#include "esp32/rom/lldesc.h"
#include "driver/periph_ctrl.h"
#include "driver/dac.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "math.h"

const float burstPerSample = (2 * M_PI) / (13333333 / 4433618.75);
const float colorFactor = (M_PI * 2) / 16;
const float burstPhase = M_PI / 4 * 3;
static lldesc_t DRAM_ATTR dma_buffers[2] = {};
static intr_handle_t i2s_interrupt_handle;
static void IRAM_ATTR i2s_interrupt(void *dma_buffer_size_bytes) {
	if (I2S0.int_st.out_eof) {
        SimplePALOutput::GetInstance()->iterator();
	}
	// reset the interrupt
    I2S0.int_clr.val = I2S0.int_st.val;
}
static void IRAM_ATTR sendline(unsigned short *l) {
    uint8_t* buf = ((uint8_t*)((lldesc_t*)I2S0.out_eof_des_addr)->buf);
    size_t bytes_to_write = lineSamples * sizeof(unsigned short);
    for (int i = 0; i < bytes_to_write; ++i) {
        buf[i] = l[i];
    }
}
SimplePALOutput::SimplePALOutput() {
    lineCounts = 1;
    for(int i = 0; i < syncSamples; i++) {
        shortSync[i ^ 1] = syncLevel << 8;
        longSync[(lineSamples - syncSamples + i) ^ 1] = blankLevel  << 8;
        line[0][i ^ 1] = syncLevel  << 8;
        line[1][i ^ 1] = syncLevel  << 8;
        blank[i ^ 1] = syncLevel  << 8;
    }
    for(int i = 0; i < lineSamples - syncSamples; i++) {
        shortSync[(i + syncSamples) ^ 1] = blankLevel  << 8;
        longSync[(lineSamples - syncSamples + i) ^ 1] = syncLevel  << 8;
        line[0][(i + syncSamples) ^ 1] = blankLevel  << 8;
        line[1][(i + syncSamples) ^ 1] = blankLevel  << 8;
        blank[(i + syncSamples) ^ 1] = blankLevel  << 8;
    }
    for(int i = 0; i < burstSamples; i++) {
        int p = burstStart + i;
        unsigned short b0 = ((short)(blankLevel + sin(i * burstPerSample + burstPhase) * burstAmp)) << 8;
        unsigned short b1 = ((short)(blankLevel + sin(i * burstPerSample - burstPhase) * burstAmp)) << 8;
        line[0][p ^ 1] = b0;
        line[1][p ^ 1] = b1;
        blank[p ^ 1] = b0;
    }

    for(int i = 0; i < imageSamples; i++) {
        int p = frameStart + i;
        int c = p - burstStart;
        SIN[i] = round(0.436798 * sin(c * burstPerSample) * 256);
        COS[i] = round(0.614777 * cos(c * burstPerSample) * 256);     
    }

    for(int i = 0; i < 16; i++) {
        YLUT[i] = (blankLevel << 8) + round(i / 15. * 256 * maxLevel);
        UVLUT[i] = round((i - 8) / 7. * maxLevel);
    }
}
void SimplePALOutput::start(Graphics *gra) {
    graph = gra;
    const size_t dma_buffer_size_bytes = lineSamples * sizeof(unsigned short);
    //ESP_LOGD(TAG, "Computed DMA buffer size: %u", dma_buffer_size_bytes);

    periph_module_enable(PERIPH_I2S0_MODULE);
    ESP_ERROR_CHECK(esp_intr_alloc(ETS_I2S0_INTR_SOURCE, ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM, i2s_interrupt, (void*)dma_buffer_size_bytes, &i2s_interrupt_handle));
    //ESP_LOGD(TAG, "I²S interrupt configured");

    // reset conf
    I2S0.conf.val = 1;
    I2S0.conf.val = 0;
    I2S0.conf.tx_right_first = 1;
    I2S0.conf.tx_mono = 1;

    I2S0.conf2.lcd_en = 1;
    I2S0.fifo_conf.tx_fifo_mod_force_en = 1;
    I2S0.sample_rate_conf.tx_bits_mod = 16; //DAC uses MSB 8 bits of 16
    I2S0.conf_chan.tx_chan_mod = 1; //Mono mode

    I2S0.clkm_conf.clkm_div_num = 1;            // I2S clock divider's integral value.
    I2S0.clkm_conf.clkm_div_b = 0;              // Fractional clock divider's numerator value.
    I2S0.clkm_conf.clkm_div_a = 1;              // Fractional clock divider's denominator value
    I2S0.sample_rate_conf.tx_bck_div_num = 1;
    I2S0.clkm_conf.clka_en = 1;                 // use clk_apll clock
    I2S0.fifo_conf.tx_fifo_mod = 1; // 16-bit single channel data

    const size_t DMA_BUFFER_COUNT = sizeof(dma_buffers)/sizeof(lldesc_t);
    for (size_t n=0; n<DMA_BUFFER_COUNT; n++) {
        //ESP_LOGD(TAG, "Allocating DMA buffer: %u bytes", dma_buffer_size_bytes);
        dma_buffers[n].buf = (uint8_t*)heap_caps_calloc(dma_buffer_size_bytes, sizeof(uint8_t), MALLOC_CAP_DMA);
        assert(dma_buffers[n].buf != NULL);
        dma_buffers[n].owner = 1;
        dma_buffers[n].eof = 1;
        dma_buffers[n].length = dma_buffer_size_bytes;
        dma_buffers[n].size = dma_buffer_size_bytes;
        dma_buffers[n].empty = (uint32_t)(n==DMA_BUFFER_COUNT-1? &dma_buffers[0] : &dma_buffers[n+1]);
    }
    I2S0.out_link.addr = (uint32_t)&dma_buffers[0];
    //this is the hack that enables the highest sampling rate possible ~13MHz, have fun
    //=13500001
    #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        rtc_clk_apll_enable(1, 0xCD, 0xCC, 0x07, 6); 
    #else
        rtc_clk_apll_enable(1);
        rtc_clk_apll_coeff_set(6, 0xCD, 0xCC, 0x07);
    #endif
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHANNEL_1));

    ESP_ERROR_CHECK(dac_i2s_enable());
    // start transmission
    I2S0.conf.tx_start = 1;
    I2S0.int_clr.val = UINT32_MAX;
    I2S0.int_ena.out_eof = 1;
    I2S0.out_link.start = 1;

    ESP_ERROR_CHECK(esp_intr_enable(i2s_interrupt_handle));
}
void SimplePALOutput::stop() {

    // disable interrupt
    //ESP_LOGD(TAG, "Disable I²S interrupt");
    ESP_ERROR_CHECK(esp_intr_disable(i2s_interrupt_handle));
    ESP_ERROR_CHECK(esp_intr_free(i2s_interrupt_handle));
    // stop DAC
    I2S0.out_link.start = 0;
    //disable i2s DAC
    //ESP_LOGD(TAG, "Disable DAC");
    dac_i2s_disable();
    dac_output_disable(DAC_CHANNEL_1);
    // free DMA buffers 
    const size_t DMA_BUFFER_COUNT = sizeof(dma_buffers)/sizeof(lldesc_t);
    for(size_t i=0;i<DMA_BUFFER_COUNT;i++) {
        if( dma_buffers[i].buf ) {
            //ESP_LOGD(TAG, "Free DMA buffers");
            heap_caps_free((uint8_t*)dma_buffers[i].buf);
            dma_buffers[i].buf=NULL;
        }
    }
    // disable I2S
    //ESP_LOGD(TAG, "Disable I²S module");
    periph_module_disable(PERIPH_I2S0_MODULE);
}
bool SimplePALOutput::iterator() {
    if(lineCounts <= 2) {
        //long sync
        sendline(longSync);
    } else if(lineCounts >= 3 && lineCounts < 7) {
        //short sync
        sendline(shortSync);
    } else if(lineCounts >= 7 && lineCounts < 44) {
        //blank lines
        sendline(blank);
    } else if(lineCounts >= 44 && lineCounts < 284){
        //make 2 lines
        int y_off = (lineCounts-44);// [0, 240)
        if (y_off%2 == 0) {
            char *pixels0 = (graph->backbuffer)[y_off];  // max: 238
            char *pixels1 = (graph->backbuffer)[y_off+1];// max: 239
            int j = frameStart;
            for(int x = 0; x < imageSamples; x += 2) {
                unsigned short p0 = *(pixels0++);
                unsigned short p1 = *(pixels1++);
                short y0 = YLUT[p0 & 15];
                short y1 = YLUT[p1 & 15];
                unsigned char p04 = p0 >> 4;
                unsigned char p14 = p1 >> 4;
                short u0 = (SIN[x] * UVLUT[p04]);
                short u1 = (SIN[x + 1] * UVLUT[p04]);
                short v0 = (COS[x] * UVLUT[p14]);
                short v1 = (COS[x + 1] * UVLUT[p14]);
                //word order is swapped for I2S packing (j + 1) comes first then j
                line[0][j] = y0 + u1 + v1;
                line[1][j] = y1 + u1 - v1;
                line[0][j + 1] = y0 + u0 + v0;
                line[1][j + 1] = y1 + u0 - v0;
                j += 2;
            }
            sendline(line[0]);
        } else {
            sendline(line[1]);
        }
    } else if(lineCounts >=284 && lineCounts < 309){
        sendline(blank);
    } else if(lineCounts >= 309 && lineCounts < 312) {
        sendline(shortSync);
    } else {
        lineCounts = 1;
        return true;
    }
    ++lineCounts;
    return false;
}