#include <gtk/gtk.h>
#include <stdio.h>

typedef struct {
  GtkWidget *window;
  GtkWidget *text_view;
  GtkWidget *open_dialog;
  GtkWidget *save_dialog;
  char *filename;
  gboolean modified;
  GtkWidget *font_selection_dialog;
  GtkWidget *status_bar;
} AppData;
AppData *app_data;

void update_status_bar(GtkTextBuffer *buffer, GtkLabel *status_bar) {
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_mark(buffer, &iter,
                                   gtk_text_buffer_get_insert(buffer));
  int line = gtk_text_iter_get_line(&iter) + 1;
  int column = gtk_text_iter_get_line_offset(&iter) + 1;

  char status_text[100];
  sprintf(status_text, "Line: %d, Column: %d", line, column);
  gtk_label_set_text(status_bar, status_text);
}

void load_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file) {
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *buffer = g_new(char, file_size + 1);
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    GtkTextBuffer *text_buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->text_view));
    gtk_text_buffer_set_text(text_buffer, buffer, -1);

    g_free(buffer);

    if (app_data->filename) {
      g_free(app_data->filename);
    }
    app_data->filename = g_strdup(filename);
    app_data->modified = FALSE;
  } else {
    g_print("Error opening file: %s\n", filename);
  }

  GtkTextBuffer *text_buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->text_view));
  GtkTextIter start;
  gtk_text_buffer_get_start_iter(text_buffer, &start);
  gtk_text_buffer_place_cursor(text_buffer, &start);

  update_status_bar(text_buffer, GTK_LABEL(app_data->status_bar));
}

void open_handler(GtkWidget *dialog, gint response_id) {
  if (response_id == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    if (filename) {
      load_file(filename);
      g_free(filename);
    }
  }

  gtk_widget_destroy(dialog);
  app_data->open_dialog = NULL;
}

void open_file() {
  if (app_data->open_dialog) {
    gtk_widget_destroy(app_data->open_dialog);
    app_data->open_dialog = NULL;
  }

  app_data->open_dialog = gtk_file_chooser_dialog_new(
      "Open File", GTK_WINDOW(app_data->window), GTK_FILE_CHOOSER_ACTION_OPEN,
      "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_do_overwrite_confirmation(
      GTK_FILE_CHOOSER(app_data->open_dialog), TRUE);

  g_signal_connect(app_data->open_dialog, "response", G_CALLBACK(open_handler),
                   NULL);

  gtk_widget_show_all(app_data->open_dialog);
}

void save_handler(GtkWidget *dialog, gint response_id) {
  if (response_id == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    if (filename) {
      GtkTextBuffer *text_buffer =
          gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->text_view));
      GtkTextIter start, end;
      gtk_text_buffer_get_start_iter(text_buffer, &start);
      gtk_text_buffer_get_end_iter(text_buffer, &end);
      char *text = gtk_text_buffer_get_text(text_buffer, &start, &end, TRUE);

      FILE *file = fopen(filename, "w");
      if (file) {
        fprintf(file, "%s", text);
        fclose(file);
        g_free(text);

        if (app_data->filename) {
          g_free(app_data->filename);
        }
        app_data->filename = g_strdup(filename);
        app_data->modified = FALSE;
      } else {
        g_print("Error saving file.\n");
        g_free(text);
      }

      g_free(filename);
    }
  }

  gtk_widget_destroy(dialog);
  app_data->save_dialog = NULL;
}

void save_as_file() {
  if (app_data->save_dialog) {
    gtk_widget_destroy(app_data->save_dialog);
    app_data->save_dialog = NULL;
  }

  app_data->save_dialog = gtk_file_chooser_dialog_new(
      "Save File As", GTK_WINDOW(app_data->window),
      GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Save",
      GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_do_overwrite_confirmation(
      GTK_FILE_CHOOSER(app_data->save_dialog), TRUE);

  g_signal_connect(app_data->save_dialog, "response", G_CALLBACK(save_handler),
                   NULL);

  gtk_widget_show_all(app_data->save_dialog);
}

void save_file() {
  if (app_data->filename) {
    GtkTextBuffer *text_buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(text_buffer, &start);
    gtk_text_buffer_get_end_iter(text_buffer, &end);
    char *text = gtk_text_buffer_get_text(text_buffer, &start, &end, TRUE);

    FILE *file = fopen(app_data->filename, "w");
    if (file) {
      fprintf(file, "%s", text);
      fclose(file);
      g_free(text);
      app_data->modified = FALSE;
    } else {
      g_print("Error saving file.\n");
      g_free(text);
    }
  } else {
    save_as_file();
  }
}

void new_file() {
  GtkTextBuffer *text_buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->text_view));
  gtk_text_buffer_set_text(text_buffer, "", -1);

  if (app_data->filename) {
    g_free(app_data->filename);
    app_data->filename = NULL;
  }
  app_data->modified = FALSE;
  update_status_bar(text_buffer, GTK_LABEL(app_data->status_bar));
}

void cleanup() {
  if (app_data->filename) {
    g_free(app_data->filename);
  }
  if (app_data->font_selection_dialog) {
    gtk_widget_destroy(app_data->font_selection_dialog);
  }
  if (app_data->open_dialog) {
    gtk_widget_destroy(app_data->open_dialog);
  }
  if (app_data->save_dialog) {
    gtk_widget_destroy(app_data->save_dialog);
  }
  g_free(app_data);
}

gboolean confirm_quit() {
  if (!app_data->modified) {
    return TRUE; // Allow window to be closed
  } else {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(app_data->window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING,
        GTK_BUTTONS_YES_NO,
        "You have unsaved changes. "
        "Do you really want to quit?");
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (response == GTK_RESPONSE_YES) {
      return TRUE; // Allow window to be closed
    } else {
      return FALSE; // Block window from closing
    }
  }
}

gboolean on_delete_event(GtkWidget *widget, GdkEvent *event) {
  if (confirm_quit()) {
    cleanup();
    gtk_main_quit();
    return FALSE; // Let the window close
  } else {
    return TRUE; // Keep the window open
  }
}

void on_text_changed(GtkWidget *widget) {
  app_data->modified = TRUE;
  update_status_bar(
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->text_view)),
      GTK_LABEL(app_data->status_bar));
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event) {
  if ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_KEY_s)) {
    save_file();
    return TRUE;
  }
  if ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_KEY_n)) {
    new_file();
    return TRUE;
  }
  if ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_KEY_o)) {
    open_file();
    return TRUE;
  }
  return FALSE;
}

gboolean on_key_release(GtkWidget *widget, GdkEventKey *event) {
  GtkTextBuffer *text_buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->text_view));
  update_status_bar(text_buffer, GTK_LABEL(app_data->status_bar));

  return FALSE;
}

void on_font_selected(GtkWidget *dialog, gint response_id) {
  if (response_id == GTK_RESPONSE_OK) {
    char *font_name = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog));
    if (font_name) {
      PangoFontDescription *font_desc =
          pango_font_description_from_string(font_name);
      if (font_desc) {
        gtk_widget_override_font(app_data->text_view, font_desc);
        pango_font_description_free(font_desc);
      } else {
        g_print("Error parsing font description.\n");
      }
      g_free(font_name);
    }
  }
  gtk_widget_destroy(dialog);
  app_data->font_selection_dialog = NULL;
}

void open_font_dialog() {
  if (app_data->font_selection_dialog) {
    gtk_widget_destroy(app_data->font_selection_dialog);
    app_data->font_selection_dialog = NULL;
  }

  app_data->font_selection_dialog =
      gtk_font_chooser_dialog_new("Select Font", GTK_WINDOW(app_data->window));

  g_signal_connect(app_data->font_selection_dialog, "response",
                   G_CALLBACK(on_font_selected), NULL);
  gtk_widget_show_all(app_data->font_selection_dialog);
}

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  app_data = g_new0(AppData, 1);
  app_data->filename = NULL;
  app_data->modified = FALSE;
  app_data->font_selection_dialog = NULL;
  app_data->open_dialog = NULL;
  app_data->save_dialog = NULL;

  app_data->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(app_data->window), "GTK Notepad");
  gtk_window_set_default_size(GTK_WINDOW(app_data->window), 600, 400);
  g_signal_connect(app_data->window, "delete-event",
                   G_CALLBACK(on_delete_event), NULL);

  gtk_window_set_resizable(GTK_WINDOW(app_data->window), FALSE);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(app_data->window), vbox);

  GtkWidget *menubar = gtk_menu_bar_new();
  gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

  GtkWidget *filemenu = gtk_menu_new();
  GtkWidget *fileitem = gtk_menu_item_new_with_label("File");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileitem), filemenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileitem);

  GtkWidget *newitem = gtk_menu_item_new_with_label("New");
  g_signal_connect_swapped(newitem, "activate", G_CALLBACK(new_file), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), newitem);

  GtkWidget *openitem = gtk_menu_item_new_with_label("Open");
  g_signal_connect_swapped(openitem, "activate", G_CALLBACK(open_file), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), openitem);

  GtkWidget *saveitem = gtk_menu_item_new_with_label("Save");
  g_signal_connect_swapped(saveitem, "activate", G_CALLBACK(save_file), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveitem);

  GtkWidget *saveasitem = gtk_menu_item_new_with_label("Save As...");
  g_signal_connect_swapped(saveasitem, "activate", G_CALLBACK(save_as_file),
                           NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveasitem);

  GtkWidget *separator = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), separator);

  GtkWidget *quititem = gtk_menu_item_new_with_label("Quit");
  g_signal_connect_swapped(quititem, "activate", G_CALLBACK(on_delete_event),
                           NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quititem);

  GtkWidget *editmenu = gtk_menu_new();
  GtkWidget *edititem = gtk_menu_item_new_with_label("Edit");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(edititem), editmenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edititem);

  GtkWidget *fontitem = gtk_menu_item_new_with_label("Font...");
  g_signal_connect_swapped(fontitem, "activate", G_CALLBACK(open_font_dialog),
                           NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), fontitem);

  app_data->text_view = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_data->text_view),
                              GTK_WRAP_WORD);
  GtkTextBuffer *text_buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->text_view));
  g_signal_connect(text_buffer, "changed", G_CALLBACK(on_text_changed), NULL);

  PangoFontDescription *font_desc =
      pango_font_description_from_string("Monospace");
  gtk_widget_override_font(app_data->text_view, font_desc);
  pango_font_description_free(font_desc);

  GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(scroll_window), app_data->text_view);

  gtk_box_pack_start(GTK_BOX(vbox), scroll_window, TRUE, TRUE, 0);

  app_data->status_bar = gtk_label_new("Line: 1, Column: 1");
  gtk_label_set_xalign(GTK_LABEL(app_data->status_bar), 0.0);
  gtk_box_pack_start(GTK_BOX(vbox), app_data->status_bar, FALSE, FALSE, 0);

  g_signal_connect(app_data->window, "key-press-event",
                   G_CALLBACK(on_key_press), NULL);
  g_signal_connect(app_data->window, "key-release-event",
                   G_CALLBACK(on_key_release), NULL);

  gtk_widget_show_all(app_data->window);

  if (argc > 1) {
    load_file(argv[1]);
  }

  gtk_main();
}
