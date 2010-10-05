/*
    Noise Sharpening plugin for DeaDBeeF
    Copyright (C) 2010 Robert Y <Decatf@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <deadbeef/deadbeef.h>


static DB_dsp_t plugin;
DB_functions_t *deadbeef;

static short enabled = 0;
static int gain = 100;

static void
dlta_reset (void);

static int
dlta_on_configchanged (DB_event_t *ev, uintptr_t data) {
    int e = deadbeef->conf_get_int ("dlta.enable", 0);
    if (e != enabled) {
        if (e) {
            dlta_reset ();
        }
        enabled = e;
    }

    int g = deadbeef->conf_get_int("dlta.gain", 100);
    if (g != gain) {
        gain = g;
    }

    return 0;
}

static int
dlta_plugin_start (void) {
    enabled = deadbeef->conf_get_int ("dlta.enable", 0);
    gain = deadbeef->conf_get_int("dlta.gain", 100);

    deadbeef->ev_subscribe (DB_PLUGIN (&plugin), DB_EV_CONFIGCHANGED, DB_CALLBACK (dlta_on_configchanged), 0);
}

static int
dlta_plugin_stop (void) {
    deadbeef->ev_unsubscribe (DB_PLUGIN (&plugin), DB_EV_CONFIGCHANGED, DB_CALLBACK (dlta_on_configchanged), 0);
}

static int
dlta_process_int16 (int16_t *samples, int nsamples, int nch, int bps, int srate) {

    if (bps != 16) {
        return nsamples;
    }

    double prevSample[nch];

    int i;
    for (i = 0; i < nch; i++) 
        prevSample[i] = samples[i];

    for (i = nch; i < nsamples*nch; i+=nch) {

        int j;
        double result;
        for (j = 0; j < nch; j++) {
            result = samples[i+j]+((gain/100.0)*(samples[i+j]-prevSample[j]));

            if (result > 32767.0) {
                result = 32767.0;
            }
            else if (result < -32768.0) {
                result = -32768.0;
            }

            prevSample[j] = samples[i+j];
            samples[i+j] = result;
        }
    }

    return nsamples;
}

static void
dlta_reset (void) {
    return;
}

static void
dlta_enable (int e) {

    if (e != enabled) {
        deadbeef->conf_set_int ("dlta.enable", e);
        if (e && !enabled) {
            dlta_reset ();
        }
        enabled = e;
    }
    return;
}

static int
dlta_enabled (void) {
    return enabled;
}

static const char settings_dlg[] =
    "property \"Enable\" checkbox dlta.enable 0;\n"
    "property \"Gain (%)\" hscale[0,200,1] dlta.gain 100;\n"
;

static DB_dsp_t plugin = {
    .plugin.api_vmajor = DB_API_VERSION_MAJOR,
    .plugin.api_vminor = DB_API_VERSION_MINOR,
    .plugin.type = DB_PLUGIN_DSP,
    .plugin.id = "delta",
    .plugin.name = "Noise Sharpening",
    .plugin.descr = "Delta DSP plugin",
    .plugin.author = "Robert Y",
    .plugin.email = "Decatf@gmail.com",
    .plugin.website = "http://deadbeef.sf.net",
    .plugin.start = dlta_plugin_start,
    .plugin.stop = dlta_plugin_stop,
    .plugin.configdialog = settings_dlg,
    .process_int16 = dlta_process_int16,
    .reset = dlta_reset,
    .enable = dlta_enable,
    .enabled = dlta_enabled,
};

DB_plugin_t *
dlta_load (DB_functions_t *api) {
    deadbeef = api;
    return DB_PLUGIN (&plugin);
}
