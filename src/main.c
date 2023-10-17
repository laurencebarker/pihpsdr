/* Copyright (C)
* 2015 - John Melton, G0ORX/N6LYT
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
*/
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wdsp.h>

#include "appearance.h"
#include "audio.h"
#include "band.h"
#include "bandstack.h"
#include "main.h"
#include "discovered.h"
#include "configure.h"
#include "actions.h"
#ifdef GPIO
  #include "gpio.h"
#endif
#include "new_menu.h"
#include "radio.h"
#include "version.h"
#include "discovery.h"
#include "new_protocol.h"
#include "old_protocol.h"
#ifdef SOAPYSDR
  #include "soapy_protocol.h"
#endif
#include "ext.h"
#include "vfo.h"
#include "css.h"
#include "exit_menu.h"
#include "message.h"

struct utsname unameData;

GdkScreen *screen;
gint display_width;
gint display_height;
gint screen_height;
gint screen_width;
gint full_screen;
gint this_monitor;

static GdkCursor *cursor_arrow;
static GdkCursor *cursor_watch;

GtkWidget *top_window;
GtkWidget *topgrid;

static GtkWidget *status_label;

void status_text(const char *text) {
  gtk_label_set_text(GTK_LABEL(status_label), text);
  usleep(100000);

  while (gtk_events_pending ()) {
    gtk_main_iteration ();
  }
}

static pthread_t wisdom_thread_id;
static int wisdom_running = 0;

static void* wisdom_thread(void *arg) {
  WDSPwisdom ((char *)arg);
  wisdom_running = 0;
  return NULL;
}

gboolean keypress_cb(GtkWidget *widget, GdkEventKey *event, gpointer data) {
  gboolean ret = TRUE;

  // Ignore key-strokes until radio is ready
  if (radio == NULL) { return FALSE; }

  //
  // Intercept key-strokes. The "keypad" stuff
  // has been contributed by Ron.
  // Everything that is not intercepted is handled downstream.
  //
  // space             ==>  MOX
  // u                 ==>  active receiver VFO up
  // d                 ==>  active receiver VFO down
  // Keypad 0..9       ==>  NUMPAD 0 ... 9
  // Keypad Decimal    ==>  NUMPAD DEC
  // Keypad Subtract   ==>  NUMPAD BS
  // Keypad Divide     ==>  NUMPAD CL
  // Keypad Multiply   ==>  NUMPAD Hz
  // Keypad Add        ==>  NUMPAD kHz
  // Keypad Enter      ==>  NUMPAD MHz
  //
  switch (event->keyval) {
  case GDK_KEY_space:
    if (can_transmit) {
      if (getTune() == 1) {
        setTune(0);
      }

      if (getMox() == 1) {
        setMox(0);
      } else if (TransmitAllowed()) {
        setMox(1);
      } else {
        transmitter_set_out_of_band(transmitter);
      }
    }

    break;

  case  GDK_KEY_d:
    vfo_step(-1);
    break;

  case GDK_KEY_u:
    vfo_step(1);
    break;

  //
  // Suggestion of Richard: using U and D for changing
  // the frequency of the "other" VFO in large steps
  // (useful for split operation)
  //
  case  GDK_KEY_U:
    vfo_id_step(1 - active_receiver->id, 10);
    break;

  case  GDK_KEY_D:
    vfo_id_step(1 - active_receiver->id, -10);
    break;

  //
  // This is a contribution of Ron, it uses a keypad for
  // entering a frequency
  //
  case GDK_KEY_KP_0:
    num_pad(0, active_receiver->id);
    break;

  case GDK_KEY_KP_1:
    num_pad(1, active_receiver->id);
    break;

  case GDK_KEY_KP_2:
    num_pad(2, active_receiver->id);
    break;

  case GDK_KEY_KP_3:
    num_pad(3, active_receiver->id);
    break;

  case GDK_KEY_KP_4:
    num_pad(4, active_receiver->id);
    break;

  case GDK_KEY_KP_5:
    num_pad(5, active_receiver->id);
    break;

  case GDK_KEY_KP_6:
    num_pad(6, active_receiver->id);
    break;

  case GDK_KEY_KP_7:
    num_pad(7, active_receiver->id);
    break;

  case GDK_KEY_KP_8:
    num_pad(8, active_receiver->id);
    break;

  case GDK_KEY_KP_9:
    num_pad(9, active_receiver->id);
    break;

  case GDK_KEY_KP_Divide:
    num_pad(-1, active_receiver->id);
    break;

  case GDK_KEY_KP_Multiply:
    num_pad(-2, active_receiver->id);
    break;

  case GDK_KEY_KP_Add:
    num_pad(-3, active_receiver->id);
    break;

  case GDK_KEY_KP_Enter:
    num_pad(-4, active_receiver->id);
    break;

  case GDK_KEY_KP_Decimal:
    num_pad(-5, active_receiver->id);
    break;

  case GDK_KEY_KP_Subtract:
    num_pad(-6, active_receiver->id);
    break;

  default:
    // not intercepted, so handle downstream
    ret = FALSE;
    break;
  }

  g_idle_add(ext_vfo_update, NULL);
  return ret;
}

gboolean main_delete (GtkWidget *widget) {
  if (radio != NULL) {
    stop_program();
  }

  _exit(0);
}

static int init(void *data) {
  char wisdom_directory[1024];
  t_print("%s\n", __FUNCTION__);
  audio_get_cards();
  cursor_arrow = gdk_cursor_new(GDK_ARROW);
  cursor_watch = gdk_cursor_new(GDK_WATCH);
  gdk_window_set_cursor(gtk_widget_get_window(top_window), cursor_watch);
  //
  // Let WDSP (via FFTW) check for wisdom file in current dir
  // If there is one, the "wisdom thread" takes no time
  // Depending on the WDSP version, the file is wdspWisdom or wdspWisdom00.
  //
  (void) getcwd(wisdom_directory, sizeof(wisdom_directory));
  strlcat(wisdom_directory, "/", 1024);
  t_print("Securing wisdom file in directory: %s\n", wisdom_directory);
  status_text("Checking FFTW Wisdom file ...");
  wisdom_running = 1;
  pthread_create(&wisdom_thread_id, NULL, wisdom_thread, wisdom_directory);

  while (wisdom_running) {
    // wait for the wisdom thread to complete, meanwhile
    // handling any GTK events.
    usleep(100000); // 100ms

    while (gtk_events_pending ()) {
      gtk_main_iteration ();
    }

    char text[1024];
    snprintf(text, 1024, "Please do not close this window until wisdom plans are completed ...\n\n... %s",
            wisdom_get_status());
    status_text(text);
  }

  //
  // When widsom plans are complete, start discovery process
  //
  g_timeout_add(100, delayed_discovery, NULL);
  return 0;
}

static void activate_pihpsdr(GtkApplication *app, gpointer data) {
  char text[256];
  t_print("Build: %s (Commit: %s, Date: %s)\n", build_version, build_commit, build_date);
  t_print("GTK+ version %u.%u.%u\n", gtk_major_version, gtk_minor_version, gtk_micro_version);
  uname(&unameData);
  t_print("sysname: %s\n", unameData.sysname);
  t_print("nodename: %s\n", unameData.nodename);
  t_print("release: %s\n", unameData.release);
  t_print("version: %s\n", unameData.version);
  t_print("machine: %s\n", unameData.machine);
  load_css();
  GdkDisplay *display = gdk_display_get_default();

  if (display == NULL) {
    t_print("no default display!\n");
    _exit(0);
  }

  screen = gdk_display_get_default_screen(display);

  if (screen == NULL) {
    t_print("no default screen!\n");
    _exit(0);
  }

  //
  // Create top window with minimum size
  //
  t_print("create top level window\n");
  top_window = gtk_application_window_new (app);
  gtk_widget_set_size_request(top_window, 100, 100);
  gtk_window_set_title (GTK_WINDOW (top_window), "piHPSDR");
  //
  // do not use GTK_WIN_POS_CENTER_ALWAYS, since this will let the
  // window jump back to the center each time the window is
  // re-created, e.g. in reconfigure_radio()
  //
  gtk_window_set_position(GTK_WINDOW(top_window), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable(GTK_WINDOW(top_window), TRUE);
  //
  // Get the position of the top window, and then determine
  // to which monitor this position belongs.
  //
  gint x, y;
  gtk_window_get_position(GTK_WINDOW(top_window), &x, &y);
  this_monitor = gdk_screen_get_monitor_at_point(screen, x, y);
  t_print("Monitor Number within Screen=%d\n", this_monitor);
  //
  // Determine the size of "our" monitor
  //
  GdkRectangle rect;
  gdk_screen_get_monitor_geometry(screen, this_monitor, &rect);
  screen_width = rect.width;
  screen_height = rect.height;
  t_print("Monitor: width=%d height=%d\n", screen_width, screen_height);
  // Start with the smallest possible screen, 800x480, since this can now
  // be fully configured in the SCREEN menu.
  // Go to "full screen" mode if display nearly matches 800x480
  // This is all overridden from the props file
  display_width  = 800;
  display_height = 480;
  full_screen    = 0;

  //
  // Go to full-screen mode by default, if the screen size is approx. 800*480
  //
  if (screen_width > 780 && screen_width < 820 && screen_height > 460 && screen_height < 500) {
    full_screen = 1;
    display_width = screen_width;
    display_height = screen_height;
  }

  t_print("display_width=%d display_height=%d\n", display_width, display_height);

  if (full_screen) {
    t_print("full screen\n");
    gtk_window_fullscreen_on_monitor(GTK_WINDOW(top_window), screen, this_monitor);
  }

  GError *error;

  if (!gtk_window_set_icon_from_file (GTK_WINDOW(top_window), "hpsdr.png", &error)) {
    t_print("Warning: failed to set icon for top_window\n");

    if (error != NULL) {
      t_print("%s\n", error->message);
    }
  }

  g_signal_connect (top_window, "delete-event", G_CALLBACK (main_delete), NULL);
  //
  // We want to use the space-bar as an alternative to go to TX
  //
  gtk_widget_add_events(top_window, GDK_KEY_PRESS_MASK);
  g_signal_connect(top_window, "key_press_event", G_CALLBACK(keypress_cb), NULL);
  t_print("create grid\n");
  topgrid = gtk_grid_new();
  gtk_widget_set_size_request(topgrid, display_width, display_height);
  gtk_grid_set_row_homogeneous(GTK_GRID(topgrid), FALSE);
  gtk_grid_set_column_homogeneous(GTK_GRID(topgrid), FALSE);
  gtk_grid_set_column_spacing (GTK_GRID(topgrid), 10);
  t_print("add grid\n");
  gtk_container_add (GTK_CONTAINER (top_window), topgrid);
  t_print("create image\n");
  GtkWidget  *image = gtk_image_new_from_file("hpsdr.png");
  t_print("add image to grid\n");
  gtk_grid_attach(GTK_GRID(topgrid), image, 0, 0, 1, 2);
  t_print("create pi label\n");
  GtkWidget *pi_label = gtk_label_new("piHPSDR by John Melton G0ORX/N6LYT");
  gtk_widget_set_name(pi_label, "big_txt");
  gtk_widget_set_halign(pi_label, GTK_ALIGN_START);
  t_print("add pi label to grid\n");
  gtk_grid_attach(GTK_GRID(topgrid), pi_label, 1, 0, 3, 1);
  t_print("create build label\n");
  snprintf(text, 256, "Built %s, Version %s\nIncludes %s", build_date, build_version, version);
  GtkWidget *build_date_label = gtk_label_new(text);
  gtk_widget_set_name(build_date_label, "med_txt");
  gtk_widget_set_halign(build_date_label, GTK_ALIGN_START);
  t_print("add build label to grid\n");
  gtk_grid_attach(GTK_GRID(topgrid), build_date_label, 1, 1, 3, 1);
  t_print("create status\n");
  status_label = gtk_label_new(NULL);
  gtk_widget_set_name(status_label, "med_txt");
  gtk_widget_set_halign(status_label, GTK_ALIGN_START);
  t_print("add status to grid\n");
  gtk_grid_attach(GTK_GRID(topgrid), status_label, 1, 2, 3, 1);
  gtk_widget_show_all(top_window);
  t_print("g_idle_add: init\n");
  g_idle_add(init, NULL);
}

int main(int argc, char **argv) {
  GtkApplication *pihpsdr;
  int rc;
  char name[1024];
#ifdef __APPLE__
  void MacOSstartup(const char *path);
  MacOSstartup(argv[0]);
#endif
  snprintf(name, 1024, "org.g0orx.pihpsdr.pid%d", getpid());
  //t_print("gtk_application_new: %s\n",name);
  pihpsdr = gtk_application_new(name, G_APPLICATION_FLAGS_NONE);
  g_signal_connect(pihpsdr, "activate", G_CALLBACK(activate_pihpsdr), NULL);
  rc = g_application_run(G_APPLICATION(pihpsdr), argc, argv);
  t_print("exiting ...\n");
  g_object_unref(pihpsdr);
  return rc;
}
