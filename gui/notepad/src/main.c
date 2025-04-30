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

int main(int argc, char *argv[]) {
}
