#include <gtk/gtk.h>
#include <stdio.h>

typedef struct {
  GtkWidget *window;
  GtkWidget *text_view;
  GtkWidget *open_dialog;
  GtkWidget *save_dialog;
  char *filename;
  gboolean modified;
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

int main(int argc, char *argv[]) {
}
