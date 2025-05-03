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
} AppData;

void load_file(AppData *data, const char *filename) {
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
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->text_view));
    gtk_text_buffer_set_text(text_buffer, buffer, -1);

    g_free(buffer);

    if (data->filename) {
      g_free(data->filename);
    }
    data->filename = g_strdup(filename);
    data->modified = FALSE;
  } else {
    g_print("Error opening file: %s\n", filename);
  }

  GtkTextBuffer *text_buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->text_view));
  GtkTextIter start;
  gtk_text_buffer_get_start_iter(text_buffer, &start);
  gtk_text_buffer_place_cursor(text_buffer, &start);
}

void open_handler(GtkWidget *dialog, gint response_id, AppData *data) {
  if (response_id == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    if (filename) {
      load_file(data, filename);
      g_free(filename);
    }
  }

  gtk_widget_destroy(dialog);
  data->open_dialog = NULL;
}

void open_file(AppData *data) {
  if (data->open_dialog) {
    gtk_widget_destroy(data->open_dialog);
  }

  data->open_dialog = gtk_file_chooser_dialog_new(
      "Open File", GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN,
      "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_do_overwrite_confirmation(
      GTK_FILE_CHOOSER(data->open_dialog), TRUE);

  g_signal_connect(data->open_dialog, "response", G_CALLBACK(open_handler),
                   data);

  gtk_widget_show_all(data->open_dialog);
}

void save_handler(GtkWidget *dialog, gint response_id, AppData *data) {
  if (response_id == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    if (filename) {
      GtkTextBuffer *text_buffer =
          gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->text_view));
      GtkTextIter start, end;
      gtk_text_buffer_get_start_iter(text_buffer, &start);
      gtk_text_buffer_get_end_iter(text_buffer, &end);
      char *text = gtk_text_buffer_get_text(text_buffer, &start, &end, TRUE);

      FILE *file = fopen(filename, "w");
      if (file) {
        fprintf(file, "%s", text);
        fclose(file);
        g_free(text);

        if (data->filename) {
          g_free(data->filename);
        }
        data->filename = g_strdup(filename);
        data->modified = FALSE;
      } else {
        g_print("Error saving file.\n");
        g_free(text);
      }

      g_free(filename);
    }
  }

  gtk_widget_destroy(dialog);
  data->save_dialog = NULL;
}

void save_as_file(AppData *data) {
  if (data->save_dialog) {
    gtk_widget_destroy(data->save_dialog);
  }

  data->save_dialog = gtk_file_chooser_dialog_new(
      "Save File As", GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE,
      "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_do_overwrite_confirmation(
      GTK_FILE_CHOOSER(data->save_dialog), TRUE);

  g_signal_connect(data->save_dialog, "response", G_CALLBACK(save_handler),
                   data);

  gtk_widget_show_all(data->save_dialog);
}

void save_file(AppData *data) {
  if (data->filename) {
    GtkTextBuffer *text_buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(text_buffer, &start);
    gtk_text_buffer_get_end_iter(text_buffer, &end);
    char *text = gtk_text_buffer_get_text(text_buffer, &start, &end, TRUE);

    FILE *file = fopen(data->filename, "w");
    if (file) {
      fprintf(file, "%s", text);
      fclose(file);
      g_free(text);
      data->modified = FALSE;
    } else {
      g_print("Error saving file.\n");
      g_free(text);
    }
  } else {
    save_as_file(data);
  }
}

void new_file(AppData *data) {
  GtkTextBuffer *text_buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->text_view));
  gtk_text_buffer_set_text(text_buffer, "", -1);

  if (data->filename) {
    g_free(data->filename);
    data->filename = NULL;
  }
  data->modified = FALSE;
}

gboolean confirm_quit(AppData *data) {
  if (!data->modified) {
    return TRUE; // Allow window to be closed
  } else {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(data->window),
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

gboolean on_delete_event(GtkWidget *widget, GdkEvent *event, AppData *data) {
  if (confirm_quit(data)) {
    gtk_main_quit();
    return FALSE; // Let the window close
  } else {
    return TRUE; // Keep the window open
  }
}

void on_text_changed(GtkWidget *widget, AppData *data) {
  data->modified = TRUE;
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, AppData *data) {
  if ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_KEY_s)) {
    save_file(data);
    return TRUE;
  }
  if ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_KEY_n)) {
    new_file(data);
    return TRUE;
  }
  if ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_KEY_o)) {
    open_file(data);
    return TRUE;
  }
  return FALSE;
}

void on_font_selected(GtkWidget *dialog, gint response_id, AppData *data) {
  if (response_id == GTK_RESPONSE_OK) {
    char *font_name = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog));
    if (font_name) {
      PangoFontDescription *font_desc =
          pango_font_description_from_string(font_name);
      if (font_desc) {
        gtk_widget_override_font(data->text_view, font_desc);
        pango_font_description_free(font_desc);
      } else {
        g_print("Error parsing font description.\n");
      }
      g_free(font_name);
    }
  }
  gtk_widget_destroy(dialog);
  data->font_selection_dialog = NULL;
}

void open_font_dialog(AppData *data) {
  if (data->font_selection_dialog) {
    gtk_widget_destroy(data->font_selection_dialog);
  }

  data->font_selection_dialog =
      gtk_font_chooser_dialog_new("Select Font", GTK_WINDOW(data->window));

  g_signal_connect(data->font_selection_dialog, "response",
                   G_CALLBACK(on_font_selected), data);
  gtk_widget_show_all(data->font_selection_dialog);
}

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  AppData *data = g_new0(AppData, 1);
  data->filename = NULL;
  data->modified = FALSE;
  data->font_selection_dialog = NULL;

  data->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(data->window), "GTK Notepad");
  gtk_window_set_default_size(GTK_WINDOW(data->window), 600, 400);
  g_signal_connect(data->window, "delete-event", G_CALLBACK(on_delete_event),
                   data);

  gtk_window_set_resizable(GTK_WINDOW(data->window), FALSE);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(data->window), vbox);

  GtkWidget *menubar = gtk_menu_bar_new();
  gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

  GtkWidget *filemenu = gtk_menu_new();
  GtkWidget *fileitem = gtk_menu_item_new_with_label("File");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileitem), filemenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileitem);

  GtkWidget *newitem = gtk_menu_item_new_with_label("New");
  g_signal_connect_swapped(newitem, "activate", G_CALLBACK(new_file), data);
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), newitem);

  GtkWidget *openitem = gtk_menu_item_new_with_label("Open");
  g_signal_connect_swapped(openitem, "activate", G_CALLBACK(open_file), data);
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), openitem);

  GtkWidget *saveitem = gtk_menu_item_new_with_label("Save");
  g_signal_connect_swapped(saveitem, "activate", G_CALLBACK(save_file), data);
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveitem);

  GtkWidget *saveasitem = gtk_menu_item_new_with_label("Save As...");
  g_signal_connect_swapped(saveasitem, "activate", G_CALLBACK(save_as_file),
                           data);
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveasitem);

  GtkWidget *separator = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), separator);

  GtkWidget *quititem = gtk_menu_item_new_with_label("Quit");
  g_signal_connect_swapped(quititem, "activate", G_CALLBACK(on_delete_event),
                           data);
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quititem);

  GtkWidget *editmenu = gtk_menu_new();
  GtkWidget *edititem = gtk_menu_item_new_with_label("Edit");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(edititem), editmenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edititem);

  GtkWidget *fontitem = gtk_menu_item_new_with_label("Font...");
  g_signal_connect_swapped(fontitem, "activate", G_CALLBACK(open_font_dialog),
                           data);
  gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), fontitem);

  data->text_view = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(data->text_view), GTK_WRAP_WORD);
  GtkTextBuffer *text_buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->text_view));
  g_signal_connect(text_buffer, "changed", G_CALLBACK(on_text_changed), data);

  PangoFontDescription *font_desc =
      pango_font_description_from_string("Monospace");
  gtk_widget_override_font(data->text_view, font_desc);
  pango_font_description_free(font_desc);

  GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(scroll_window), data->text_view);

  gtk_box_pack_start(GTK_BOX(vbox), scroll_window, TRUE, TRUE, 0);

  g_signal_connect(data->window, "key-press-event", G_CALLBACK(on_key_press),
                   data);

  gtk_widget_show_all(data->window);

  if (argc > 1) {
    load_file(data, argv[1]);
  }

  gtk_main();
}
