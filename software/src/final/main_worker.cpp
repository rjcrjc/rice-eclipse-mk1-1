//
// Created by rjcunningham on 12/1/17.
//

#include <assert.h>
#include <iostream>
#include "unistd.h"
#include "main_worker.hpp"
#include "../util/timestamps.hpp"
#include "pins.hpp"

#define SEND_TIME 100000 //Send every 100ms.

network_queue_item null_nqi = {nq_none}; //An item for null args to
work_queue_item null_wqi = {wq_none}; //An object with the non-matching type to do nothing.

adc_reading adcd = {};

static int ti_count = 0;
static timed_item ti_list[MAX_TIMED_LIST_LEN] = {};

static void add_timed_item(timed_item &ti) {
    int i;
    for (i = 0; i < MAX_TIMED_LIST_LEN; i++) {
        if (ti_list[i].a == wq_none) {
            ti_list[i] = ti;
            ti_count++;
            return;
        }
    }
}

static void check_ti_list(timestamp_t t, safe_queue<work_queue_item> &qw) {
    int i, ti_seen = 0;
    work_queue_item wqi;
    for (i = 0; i < MAX_TIMED_LIST_LEN && ti_seen < ti_count; i++) {
        if (ti_list[i].a != wq_none) {
            ti_seen++;
            if (ti_list[i].enabled && t > ti_list[i].scheduled && t - ti_list[i].scheduled > ti_list[i].delay) {
                // Add this to the list of items to process:
                wqi.action = wq_timed;
                wqi.extra_datap = (void *) &ti_list[i];
                qw.enqueue(wqi);
            }
        }
    }
    return;
}

static void enable_ti_item(timed_item *ti, timestamp_t now) {
    ti->scheduled = now;
    ti->enabled = true;
    std::cout << "Enabling timed item." << ti->a << std::endl;
}

static void disable_ti_item(timed_item *ti) {
    ti->enabled = false;
}

static timed_item lc_main_ti = {
        0,
        LC_MAIN_T,
        NULL,
        {

        },
        lc_main,
        true,
};

static timed_item lc1_ti = {
        0,
        LC1_T,
        NULL,
        {

        },
        lc1,
        true,
};

static timed_item lc2_ti = {
        0,
        LC2_T,
        NULL,
        {

        },
        lc2,
        true,
};

static timed_item lc3_ti = {
        0,
        LC3_T,
        NULL,
        {

        },
        lc3,
        true,
};

static timed_item pt_feed_ti = {
        0,
        PT_FEED_T,
        NULL,
        {

        },
        pt_feed,
        true,
};

static timed_item pt_inje_ti = {
        0,
        PT_INJE_T,
        NULL,
        {

        },
        pt_inje,
        true,
};

static timed_item pt_comb_ti = {
        0,
        PT_COMB_T,
        NULL,
        {

        },
        pt_comb,
        true,
};

static timed_item tc1_ti = {
        0,
        TC1_T,
        NULL,
        {

        },
        tc1,
        true,
};

static timed_item tc2_ti = {
        0,
        TC2_T,
        NULL,
        {

        },
        tc2,
        true,
};

static timed_item tc3_ti = {
        0,
        TC3_T,
        NULL,
        {

        },
        tc3,
        true,
};

static timed_item ign2_ti = {
        0,
        IGN2_T,
        NULL,
        {
               // Leave ADC info as zero.
        },
        ign2,
        false,
};

static timed_item ign3_ti = {
        0,
        IGN3_T,
        NULL,
        {
                // Leave ADC info as zero.
        },
        ign3,
        false,
};

uint16_t count = 0;

void main_worker::worker_method() {
    network_queue_item nqi;
    work_queue_item wqi;
    char c;
    bool sending = false;
    size_t last_send = 0;
    uint16_t adc_result = 0;


    timestamp_t now = get_time();
    size_t buff_size = 2 << 18;

    lc_main_ti.b = new circular_buffer(buff_size);
    lc_main_ti.scheduled = now;
    lc_main_ti.last_send = now;
    lc_main_ti.ai.pin = LC_ADC;
    lc_main_ti.ai.single_channel = true;
    lc_main_ti.ai.channel = 0;
    ti_list[0] = lc_main_ti;

    //if (lc_main_ti.b != NULL) {
    //    std::cout << "Trying to read from adc." << std::endl;
    //}

    lc1_ti.b = new circular_buffer(buff_size);
    lc1_ti.scheduled = now;
    lc1_ti.last_send = now;
    lc1_ti.ai.pin = LC_ADC;
    lc1_ti.ai.single_channel = true;
    lc1_ti.ai.channel = 1;
    ti_list[1] = lc1_ti;

    lc2_ti.b = new circular_buffer(buff_size);
    lc2_ti.scheduled = now;
    lc2_ti.last_send = now;
    lc2_ti.ai.pin = LC_ADC;
    lc2_ti.ai.single_channel = true;
    lc2_ti.ai.channel = 2;
    ti_list[2] = lc2_ti;

    lc3_ti.b = new circular_buffer(buff_size);
    lc3_ti.scheduled = now;
    lc3_ti.last_send = now;
    lc3_ti.ai.pin = LC_ADC;
    lc3_ti.ai.single_channel = true;
    lc3_ti.ai.channel = 3;
    ti_list[3] = lc3_ti;

    pt_inje_ti.b = new circular_buffer(buff_size);
    pt_inje_ti.scheduled = now;
    pt_inje_ti.last_send = now;
    pt_inje_ti.ai.pin = PT_ADC;
    pt_inje_ti.ai.single_channel = true;
    pt_inje_ti.ai.channel = 1;
    ti_list[4] = pt_inje_ti;

    pt_comb_ti.b = new circular_buffer(buff_size);
    pt_comb_ti.scheduled = now;
    pt_comb_ti.last_send = now;
    pt_comb_ti.ai.pin = PT_ADC;
    pt_comb_ti.ai.single_channel = true;
    pt_comb_ti.ai.channel = 2;
    ti_list[5] = pt_comb_ti;

    pt_feed_ti.b = new circular_buffer(buff_size);
    pt_feed_ti.scheduled = now;
    pt_feed_ti.last_send = now;
    pt_feed_ti.ai.pin = PT_ADC;
    pt_feed_ti.ai.single_channel = true;
    pt_feed_ti.ai.channel = 0;
    ti_list[6] = pt_feed_ti;

    tc1_ti.b = new circular_buffer(buff_size);
    tc1_ti.scheduled = now;
    tc1_ti.last_send = now;
    tc1_ti.ai.pin = TC_ADC;
    tc1_ti.ai.single_channel = true;
    tc1_ti.ai.channel = 4;
    ti_list[7] = tc1_ti;

    tc2_ti.b = new circular_buffer(buff_size);
    tc2_ti.scheduled = now;
    tc2_ti.last_send = now;
    tc2_ti.ai.pin = TC_ADC;
    tc2_ti.ai.single_channel = true;
    tc2_ti.ai.channel = 5;
    ti_list[8] = tc2_ti;

    tc3_ti.b = new circular_buffer(buff_size);
    tc3_ti.scheduled = now;
    tc3_ti.last_send = now;
    tc3_ti.ai.pin = TC_ADC;
    tc3_ti.ai.single_channel = true;
    tc3_ti.ai.channel = 6;
    ti_list[9] = tc3_ti;

    ign2_ti.b = NULL;
    ign2_ti.scheduled = now;
    ign2_ti.last_send = now;
    ti_list[10] = ign2_ti;

    ign3_ti.b = NULL;
    ign3_ti.scheduled = now;
    ign3_ti.last_send = now;
    ti_list[11] = ign3_ti;

    // TODO should be using TI add for this. Make this into a C++ object with initialization.
    ti_count = 12;

    while (1) {
        assert(ti_list[0].b != NULL);
        //std::cout << "Backworker entering loop:\n";
        wqi = qw.poll();
        //std::cout << "Backworker got item:\n";
        timestamp_t now = get_time();
        switch (wqi.action) {
            case (wq_process): {
                c = wqi.data[0];

                std::cout << "Processing request on worker: " << c << std::endl;

                switch (c) {
                    case '0': {
                        wqi.action = wq_start;
                        qw.enqueue(wqi);
                        break;
                    }
                    case '1': {
                        wqi.action = wq_stop;
                        qw.enqueue(wqi);
                        break;
                    }
                    case unset_valve: {
                        std::cout << "Writing main valve off" << std::endl;
                        bcm2835_gpio_write(MAIN_VALVE, LOW);
                        break;
                    }
                    case set_valve: {
                        std::cout << "Writing main valve on" << std::endl;
                        bcm2835_gpio_write(MAIN_VALVE, HIGH);
                        break;
                    }
                    case unset_ignition: {
                        std::cout << "Writing ignition off" << std::endl;
                        bcm2835_gpio_write(IGN_START, LOW);
                        break;
                    }
                    case set_ignition: {
                        std::cout << "Writing ignition on" << std::endl;
                        bcm2835_gpio_write(IGN_START, HIGH);
                        break;
                    }
                    case ign_normal: {
                        std::cout << "Beginning timed ignition process" << std::endl;
                        wqi.action = ign1;
                        qw.enqueue(wqi);
                        break;
                    }
                    default: {
                        wqi.action = wq_none;
                        qw.enqueue(wqi);
                        break;
                    }
                }
                break;
            }

            case (wq_stop) :{
                sending = false;
                break;
            }
            case (wq_start): {
                sending = true;
                break;
            }
            case (wq_timed): {
                // Get the timed item that added this:
                timed_item *ti = (timed_item *) wqi.extra_datap;

                ti->scheduled = now;

                
                if (ti->b != NULL) {
                    std::cout << "Reading adc please work" << ti->ai.pin << " " 
                                << ti->ai.single_channel << " " << ti->ai.channel << std::endl; 
                    adcd.dat = adcs.read_item(ti->ai);
                    //adcd.dat = count++;
                    //usleep(100);
                    //FIXME switch this.

                    adcd.t = now;
                    ti->b->add_data(&adcd, sizeof(adcd));
                    // Now see if it has been long enough that we should send data:
                    if (now - ti->last_send > SEND_TIME) {
                        size_t bw = ti->b->bytes_written.load();
                        //std::cout << "Will do sending" << std::endl;

                        //Send some data:
                        nqi.type = nq_send;
                        nqi.nbytes = bw - ti->nbytes_last_send;
                        nqi.total_bytes = ti->nbytes_last_send;
                        nqi.data[0] = ti->a; // Include info on which buffer this is from.
                        nqi.buff = ti->b;
                        ti->nbytes_last_send = bw;
                        ti->last_send = now;

                        // TODO only enqueue if the nw is connected:
                        if (!nw_ref->connected && nqi.nbytes > 0) {
                            break;
                        }
                        qn.enqueue(nqi);
                    }
                } else {
                    // Handle the case of using ignition stuff.
                    if (ti->a == ign2) {
                        //TODO allow it to enable without just some magic number for list entry.
                        disable_ti_item(&ti_list[10]);
                        //enable_ti_item(&ign3_ti, now);
                        enable_ti_item(&ti_list[11], now);
                        std::cout << "Writing main valve on" << std::endl;
                        bcm2835_gpio_write(MAIN_VALVE, HIGH);
                        break;
                        // Enable the second igntion thing:
                        // TODO
                    }
                    if (ti->a == ign3) {
                        std::cout << "Writing main valve off" << std::endl;
                        bcm2835_gpio_write(MAIN_VALVE, LOW);

                        // Disable ignition 3.
                        disable_ti_item(&ti_list[11]);

                        std::cout << "Writing ignition off" << std::endl;
                        bcm2835_gpio_write(IGN_START, LOW);
                        break;
                    }
                }
                break;
            }
            case ign1: {
                // Set the ignition on and then enable ign2.
                std::cout << "Writing ignition on" << std::endl;
                bcm2835_gpio_write(IGN_START, HIGH);

                enable_ti_item(&ti_list[10], now);
                break;
            }
            case (wq_none): {
                check_ti_list(now, qw);
                break;
            }

            default: {
                std::cerr << "Work queue item not hanlded." << std::endl;
                break;
            }

        }
    }

}
