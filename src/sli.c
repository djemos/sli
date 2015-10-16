/*	- added support to format partitions using different filesystems (btrfs,ext2,ext3,ext4,reiserfs,jfs,xfs)
	- display a progress bar instead of (non user friendly) log
	- added support for setting /home user directory on a different partition 
	- adeed support for creating a user name and user password on installation system 
	- added partition manager connectivity in gui
	- added support for core, basic, full mode installation
	*/
	
#include <gtk/gtk.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#define N_(String) String
#include "config.h"

#include "sli.h"

void do_action (gboolean copy) {
	gchar *commandline, **command, *output, *home, *fstype;
	GtkComboBox *listwidget;
	GtkTreeIter iter;
	GtkListStore *list;
	char *usernam,*userpasswd, *installation_mode, *rootdirectory, *bootloader, *format_home;
	
	GtkWidget *username;
	GtkWidget *userpassword;
	GtkWidget *core, *basic, *full;
	core = (GtkWidget *) gtk_builder_get_object(widgetstree, "core");
	basic = (GtkWidget *) gtk_builder_get_object(widgetstree, "basic");
	full = (GtkWidget *) gtk_builder_get_object(widgetstree, "full");
	
	username = (GtkWidget *) gtk_builder_get_object(widgetstree, "username");
	userpassword = (GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword");
	
	usernam =  g_strdup (gtk_entry_get_text(GTK_ENTRY(username)));
	userpasswd =  g_strdup (gtk_entry_get_text(GTK_ENTRY(userpassword)));
	
	const gchar *DW[] = { "installdevices", "copydevices" };
	//
	listwidget = (GtkComboBox *) gtk_builder_get_object(widgetstree, "homedevices");
	gtk_combo_box_get_active_iter(listwidget, &iter);
	list = (GtkListStore *) gtk_combo_box_get_model(listwidget);
	gtk_tree_model_get((GtkTreeModel *) list, &iter, 0, &home, -1);
	if (strlen(home) == 0) {
		g_free(home);
		home = g_strdup(location);
	 //	gtk_toggle_button_set_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "format_home"),FALSE);           
	}
	/*else {
		gtk_toggle_button_set_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "format_home"),TRUE);
	}*/
	//
	listwidget = (GtkComboBox *) gtk_builder_get_object(widgetstree, "filesystem");
	gtk_combo_box_get_active_iter(listwidget, &iter);
	list = (GtkListStore *) gtk_combo_box_get_model(listwidget);
	gtk_tree_model_get((GtkTreeModel *) list, &iter, 0, &fstype, -1);
	if (strlen(fstype) == 0) {
		g_free(fstype);
		fstype = g_strdup("ext4");
	}

	//
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (core))) {
		installation_mode = g_strdup ("core") ;
		rootdirectory = g_strdup ("modules");
		} 
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (basic))) {
     installation_mode = g_strdup ("basic") ;
     rootdirectory = g_strdup ("modules");
		}
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (full))) {
		installation_mode = g_strdup ("full") ;
		rootdirectory = g_strdup ("modules");
		//rootdirectory = g_strdup ("system");
		}
	
	fullpercent = FALSE;
	pulsebar = TRUE;
	progressbar_handler_id = g_timeout_add(100, progressbar_handler, NULL);
	if (location != NULL) {
		g_free(location);
	}
	listwidget = (GtkComboBox *) gtk_builder_get_object(widgetstree, DW[(guint) copy]);
	gtk_combo_box_get_active_iter(listwidget, &iter);
	list = (GtkListStore *) gtk_combo_box_get_model(listwidget);
	gtk_tree_model_get((GtkTreeModel *) list, &iter, 0, &location, -1);
	if (location == NULL) {
		return;
	}
	if (gtk_toggle_button_get_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "lilo"))) {
		bootloader = g_strdup ("lilo");
	   } 
	else if (gtk_toggle_button_get_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "grub"))){
		bootloader = g_strdup ("grub");
	 }
	
	if (gtk_toggle_button_get_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "format_home"))) {
		format_home = g_strdup ("yes");
		}
	else {
	    format_home = g_strdup ("no");
	   } 		
	
	if (copy) {
		g_spawn_command_line_sync("du -s -m /live/media", &output, NULL, NULL, NULL);
		totalsize = g_ascii_strtoull(output, NULL, 10);
		commandline = g_strdup_printf("build-slackware-live.sh --usb /live/media %s\n", location);
	} else {
			if (gtk_toggle_button_get_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "lilo"))) {
					g_spawn_command_line_sync("du -s -m /live/modules", &output, NULL, NULL, NULL);
					totalsize = g_ascii_strtoull(output, NULL, 10);
					commandline = g_strdup_printf("build-slackware-live.sh --install /live/%s %s -auto %s %s %s %s %s %s %s\n", rootdirectory, location, usernam, userpasswd, installation_mode, bootloader, format_home, fstype, home);			
			} 
			else if (gtk_toggle_button_get_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "grub"))) {
					g_spawn_command_line_sync("du -s -m /live/modules", &output, NULL, NULL, NULL);
					totalsize = g_ascii_strtoull(output, NULL, 10);
					commandline = g_strdup_printf("build-slackware-live.sh --install /live/%s %s -auto %s %s %s %s %s %s %s\n", rootdirectory, location, usernam, userpasswd, installation_mode, bootloader, format_home, fstype, home);			
			} else { 
					g_spawn_command_line_sync("du -s -m /live/modules", &output, NULL, NULL, NULL);
					totalsize = g_ascii_strtoull(output, NULL, 10);
					commandline = g_strdup_printf("build-slackware-live.sh --install /live/%s %s -expert %s %s %s %s %s %s\n", rootdirectory, location, usernam, userpasswd, installation_mode, format_home, fstype, home);
			 }
	       }

	g_shell_parse_argv(commandline, NULL, &command, NULL);
	g_free(commandline);
	g_spawn_async(NULL, command, NULL, G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid, NULL);
	g_child_watch_add(pid, on_process_end, NULL);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "copy_btn"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "install_btn"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "cancel_btn"), TRUE);
	
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "username"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword1"), FALSE);

	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "checkbutton_user"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "gparted"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "grub"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "lilo"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "format_home"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "filesystem"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "core"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "basic"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "full"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "copydevices"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "installdevices"), FALSE);
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "homedevices"), FALSE);
	
	g_strfreev(command);
}

void on_checkbutton_user_toggled (GtkWidget *widget, gpointer user_data) {
 	if (gtk_toggle_button_get_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "checkbutton_user"))) {
		gtk_entry_set_visibility (GTK_ENTRY ((GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword")), TRUE);
		gtk_entry_set_visibility (GTK_ENTRY ((GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword1")), TRUE);
	} else {
		gtk_entry_set_visibility (GTK_ENTRY ((GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword")), FALSE);
		gtk_entry_set_visibility (GTK_ENTRY ((GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword1")), FALSE);
		}	
}                           

void clearlocations() {
	GtkComboBox *listwidget;
	GtkListStore *list;
	// clear copydevices
	listwidget = (GtkComboBox *) gtk_builder_get_object(widgetstree, "copydevices");
	list = (GtkListStore *) gtk_combo_box_get_model(listwidget);
	gtk_list_store_clear (list) ;
    // Clear installdevices
	gtk_list_store_clear (list) ;
	listwidget = (GtkComboBox *) gtk_builder_get_object(widgetstree, "installdevices");
	list = (GtkListStore *) gtk_combo_box_get_model(listwidget);
	gtk_list_store_clear (list) ;
	// Clear homedevices
	gtk_list_store_clear (list) ;
	listwidget = (GtkComboBox *) gtk_builder_get_object(widgetstree, "homedevices");
	list = (GtkListStore *) gtk_combo_box_get_model(listwidget);
	gtk_list_store_clear (list) ;
	
}

void on_gparted_clicked (GtkWidget *widget, gpointer user_data) {
	if (g_file_test("/usr/sbin/gparted", G_FILE_TEST_EXISTS)) {
	system("/usr/sbin/gparted");
	clearlocations();
	initlocations();
	}
}

void on_copy_btn_clicked (GtkWidget *widget, gpointer user_data) {
	do_action(TRUE);
}

void on_install_btn_clicked (GtkWidget *widget, gpointer user_data) {
	GtkWidget *dialog;

	GtkWidget *username;
	GtkWidget *userpassword;

	GtkWidget *userpassword1;
	
	gchar *fstype;
	GtkComboBox *listwidget;
	GtkTreeIter iter;
	GtkListStore *list;
		
	username = (GtkWidget *) gtk_builder_get_object(widgetstree, "username");
	userpassword = (GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword");
    
	userpassword1 = (GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword1");
	
	listwidget = (GtkComboBox *) gtk_builder_get_object(widgetstree, "filesystem");
	gtk_combo_box_get_active_iter(listwidget, &iter);
	list = (GtkListStore *) gtk_combo_box_get_model(listwidget);
	gtk_tree_model_get((GtkTreeModel *) list, &iter, 0, &fstype, -1);
	
	if (gtk_entry_get_text_length (GTK_ENTRY(username)) == 0
			|| (gtk_entry_get_text_length (GTK_ENTRY(userpassword)) == 0)) {
				dialog = (GtkWidget *) gtk_builder_get_object(widgetstree, "dialogusers");
				gtk_widget_show(dialog);
	       }
	 else if (gtk_entry_get_text_length (GTK_ENTRY(userpassword)) < 5) {
		 	    dialog = (GtkWidget *) gtk_builder_get_object(widgetstree, "dialogusers");
				gtk_widget_show(dialog);
	       }  
    else if  (strcmp(gtk_entry_get_text (GTK_ENTRY(userpassword)),gtk_entry_get_text (GTK_ENTRY(userpassword1)))!=0 ) {
				dialog = (GtkWidget *) gtk_builder_get_object(widgetstree, "dialoguserpass");
				gtk_widget_show(dialog);				
		   }
    else if (gtk_toggle_button_get_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "lilo")) 
		&& gtk_toggle_button_get_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "grub"))){ 
			dialog = (GtkWidget *) gtk_builder_get_object(widgetstree, "dialogbootloader");
			gtk_widget_show(dialog);}
	else if (gtk_toggle_button_get_active((GtkToggleButton*) gtk_builder_get_object(widgetstree, "grub")) && (strcmp(fstype,"xfs") == 0)){
		dialog = (GtkWidget *) gtk_builder_get_object(widgetstree, "dialog_grub_xfs");
		gtk_widget_show(dialog);}	
	else {
	do_action(FALSE);
	}
}

void on_cancel_btn_clicked (GtkWidget *widget, gpointer user_data) {
	kill (pid, SIGTERM);
}


void on_exit (GtkWidget *widget, gpointer user_data) {
	if (pid != 0) {
		kill (pid, SIGTERM);
	}
	gtk_main_quit();
}

gboolean progressbar_handler(gpointer data) {
	GtkProgressBar *progressbar;
	gchar *output;
	guint64 installsize;
	gdouble progressfraction;
	gchar *s_progressfraction;
	
	progressbar = (GtkProgressBar *) gtk_builder_get_object(widgetstree,"progressbar");
	
	if (pulsebar && ! fullpercent && g_file_test("/mnt/install",  G_FILE_TEST_IS_DIR)) {
		pulsebar = FALSE;
		g_source_remove(progressbar_handler_id);
		progressbar_handler_id = g_timeout_add(5000, progressbar_handler, NULL);
	}
	
	if (pulsebar) {
		gtk_progress_bar_pulse(progressbar);
	} else {
		g_spawn_command_line_sync("du -s -m /mnt/install", &output, NULL, NULL, NULL);
		installsize = g_ascii_strtoull(output, NULL, 10);
		progressfraction = installsize * 100 / totalsize;
		if (progressfraction >= 100) {
			gtk_progress_bar_set_text(progressbar, "100 %");
			gtk_progress_bar_set_fraction(progressbar, 1.0);
			fullpercent = TRUE;
			pulsebar = TRUE;
			g_source_remove(progressbar_handler_id);
			progressbar_handler_id = g_timeout_add(100, progressbar_handler, NULL);
		} else {
			gtk_progress_bar_set_fraction(progressbar, progressfraction / 100);
			s_progressfraction = g_strdup_printf("%2.0f %c", progressfraction, '%');
			gtk_progress_bar_set_text(progressbar, s_progressfraction);
			g_free(s_progressfraction);
		}
	}
	return TRUE;
}


void initlocations() {
	GtkComboBox *listwidget;
	GtkTreeIter iter;
	GtkListStore *list;
	gchar **lines, *output, *device;
	gint i;
	gint status;
	
	listwidget = (GtkComboBox *) gtk_builder_get_object(widgetstree, "copydevices");
	list = (GtkListStore *) gtk_combo_box_get_model(listwidget);
	copydevicescount = 0;
	g_spawn_command_line_sync("sli-location-detection.sh copy", &output, NULL, &status, NULL);
	if (status == 0) {
		lines = g_strsplit(output, "\n", 0);
		for (i=0; lines[i] != NULL && strlen(lines[i])>0; i++) {
			gtk_list_store_append(list, &iter);
			gtk_list_store_set(list, &iter, 0, lines[i], -1);
		}
		copydevicescount = i;
		g_strfreev(lines);
	}
	g_free(output);
	if (copydevicescount != 0){
		gtk_combo_box_set_active_iter(listwidget, &iter);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "copy_btn"), TRUE);
	}
	
	
	listwidget = (GtkComboBox *) gtk_builder_get_object(widgetstree, "installdevices");
	list = (GtkListStore *) gtk_combo_box_get_model(listwidget);
	installdevicescount = 0;
	g_spawn_command_line_sync("sli-location-detection.sh install", &output, NULL, &status, NULL);
	if (status == 0) {
		lines = g_strsplit(output, "\n", 0);
		for (i=0; lines[i] != NULL && strlen(lines[i])>0; i++) {
			gtk_list_store_append(list, &iter);
			gtk_list_store_set(list, &iter, 0, lines[i], -1);
		}
		installdevicescount = i;
		g_strfreev(lines);
	}
	g_free(output);
	if (installdevicescount != 0){
		gtk_combo_box_set_active_iter(listwidget, &iter);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "install_btn"), TRUE);
	}

	listwidget = (GtkComboBox *) gtk_builder_get_object(widgetstree, "homedevices");
	list = (GtkListStore *) gtk_combo_box_get_model(listwidget);
	homedevicescount = 0;
	g_spawn_command_line_sync("sli-location-detection.sh home", &output, NULL, &status, NULL);
	if (status == 0) {
		lines = g_strsplit(output, "\n", 0);
		for (i=0; lines[i] != NULL && strlen(lines[i])>0; i++) {
			gtk_list_store_append(list, &iter);
			gtk_list_store_set(list, &iter, 0, lines[i], -1);
		}
		homedevicescount = i;
		g_strfreev(lines);
		gtk_list_store_prepend(list, &iter);
		gtk_list_store_set(list, &iter, 0, "", -1);
	}
	g_free(output);
	if (homedevicescount != 0){
		gtk_combo_box_set_active_iter(listwidget, &iter);
	}
}


void on_process_end (GPid thepid, gint status, gpointer data) {
	GtkWidget *dialog;
	GtkProgressBar *progressbar;

	pid = 0;
	g_free(location);
	location = NULL;
	g_source_remove(progressbar_handler_id);
	
	progressbar = (GtkProgressBar *) gtk_builder_get_object(widgetstree,"progressbar");
	gtk_progress_bar_set_fraction(progressbar, 0);
	gtk_progress_bar_set_text(progressbar, "");

	if (copydevicescount != 0){
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "copy_btn"), TRUE);
	}
	if (installdevicescount != 0){
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "install_btn"), TRUE);
		
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "username"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "userpassword1"), TRUE);

		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "checkbutton_user"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "gparted"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "gparted"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "grub"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "lilo"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "format_home"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "filesystem"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "core"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "basic"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "full"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "copydevices"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "installdevices"), TRUE);
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "homedevices"), TRUE);
	}
	gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object(widgetstree, "cancel_btn"), FALSE);
	
	if (0 == status) {
		dialog = (GtkWidget *) gtk_builder_get_object(widgetstree, "dialogfinished");
	} else {
		dialog = (GtkWidget *) gtk_builder_get_object(widgetstree, "dialogerror");
	}
	
	gtk_widget_show(dialog);
}


void on_about_activate (GtkWidget *widget, gpointer user_data) {
	GtkWidget *aboutdialog;
	
	aboutdialog = (GtkWidget *) gtk_builder_get_object(widgetstree, "aboutdialog");
	gtk_widget_show(aboutdialog);
}

void on_quit_activate (GtkWidget *widget, gpointer user_data) {
	if (pid != 0) {
		kill (pid, SIGTERM);
	}
	gtk_main_quit();
}


int main (int argc, char *argv[]) {
	GtkWindow *mainwindow;
	GtkWidget *notlivedialog;
	GtkWidget *menubar1;
	GtkWidget *about;
	GtkAboutDialog *aboutdialog;
	gchar *path;
		
	setlocale(LC_ALL, "");
	bindtextdomain(PROJECT_NAME, LOCALE_DIR);
	textdomain(PROJECT_NAME);
	
	path = g_strdup_printf("/sbin:/usr/sbin:/usr/local/sbin:%s", g_getenv("PATH"));
	g_setenv("PATH", path, TRUE);
	g_free(path);
	
	gtk_init(&argc, &argv);
	widgetstree = gtk_builder_new();
	gtk_builder_add_from_file(widgetstree, UI_FILE, NULL);
	gtk_builder_connect_signals(widgetstree, NULL);
	
	pid = 0;
	location = NULL;
		
	mainwindow = (GtkWindow *) gtk_builder_get_object(widgetstree, "mainwindow");
	gtk_window_set_icon_from_file(mainwindow, APP_ICON, NULL);
	aboutdialog = (GtkAboutDialog *) gtk_builder_get_object(widgetstree, "aboutdialog");
	
	gtk_about_dialog_set_version(aboutdialog, PROJECT_VERSION);
	gtk_widget_show((GtkWidget *) mainwindow);
	
	if (g_file_test("/live/media/boot/liveboot", G_FILE_TEST_EXISTS)) {
		initlocations();
	} else {
		notlivedialog = (GtkWidget *) gtk_builder_get_object(widgetstree, "notlivedialog");
		gtk_widget_show(notlivedialog);
	}	
	gtk_main();
	return 0;
}
