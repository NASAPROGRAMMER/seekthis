#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aes.h" 

GtkWidget *entry_key;
GtkWidget *file_chooser;
GtkWidget *label_status;

void aes_encrypt_file(const char *filename, const char *key_str) {
    FILE *f = fopen(filename, "rb+");
    if (!f) {
        gtk_label_set_text(GTK_LABEL(label_status), "Fail to open file!");
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    uint8_t *data = malloc(size + AES_BLOCKLEN); // untuk padding
    fread(data, 1, size, f);

    // PKCS7 Padding
    uint8_t pad_len = AES_BLOCKLEN - (size % AES_BLOCKLEN);
    for (int i = 0; i < pad_len; i++) {
        data[size + i] = pad_len;
    }

    long total_size = size + pad_len;

    // Siapkan key dan iv
    uint8_t key[16] = {0};
    strncpy((char*)key, key_str, 16);
    uint8_t iv[16] = {0}; // bisa diganti dengan random IV

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, data, total_size);

    rewind(f);
    fwrite(data, 1, total_size, f);
    fclose(f);
    free(data);
}

void aes_decrypt_file(const char *filename, const char *key_str) {
    FILE *f = fopen(filename, "rb+");
    if (!f) {
        gtk_label_set_text(GTK_LABEL(label_status), "Fail to open file!");
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    uint8_t *data = malloc(size);
    fread(data, 1, size, f);

    // Siapkan key dan iv
    uint8_t key[16] = {0};
    strncpy((char*)key, key_str, 16);
    uint8_t iv[16] = {0}; // harus sama dengan yang dipakai saat enkripsi

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_decrypt_buffer(&ctx, data, size);

    // Hapus padding
    uint8_t pad_len = data[size - 1];
    long unpadded_size = size - pad_len;

    rewind(f);
    fwrite(data, 1, unpadded_size, f);
    ftruncate(fileno(f), unpadded_size); // potong file sesuai ukuran
    fclose(f);
    free(data);
}

void on_encrypt_clicked(GtkButton *button, gpointer user_data) {
    const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
    const char *key = gtk_entry_get_text(GTK_ENTRY(entry_key));

    if (!filename || strlen(key) == 0) {
        gtk_label_set_text(GTK_LABEL(label_status), "File or key empty!");
        return;
    }

    aes_encrypt_file(filename, key);
    gtk_label_set_text(GTK_LABEL(label_status), "File succesfully encryption!");
    g_free((gchar *)filename);
}

void on_decrypt_clicked(GtkButton *button, gpointer user_data) {
    const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
    const char *key = gtk_entry_get_text(GTK_ENTRY(entry_key));

    if (!filename || strlen(key) == 0) {
        gtk_label_set_text(GTK_LABEL(label_status), "File or key empty!");
        return;
    }

    aes_decrypt_file(filename, key);
    gtk_label_set_text(GTK_LABEL(label_status), "File succesfully decryption!");
    g_free((gchar *)filename);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Seekthis by ranggaxr - Hide your important files safe");
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "Seekthis.png", NULL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 20);
    gtk_container_add(GTK_CONTAINER(window), grid);

    file_chooser = gtk_file_chooser_button_new("Choose File", GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_grid_attach(GTK_GRID(grid), file_chooser, 0, 0, 2, 1);

    entry_key = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_key), "Enter key (max 16 character)");
    gtk_grid_attach(GTK_GRID(grid), entry_key, 0, 1, 2, 1);
    
    GtkWidget *label_note = gtk_label_new("Note: Never close the program while process is happen!"); 
    gtk_label_set_xalign(GTK_LABEL(label_note), 0.0); // rata kiri
    gtk_widget_set_halign(label_note, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label_note, 0, 2, 2, 1);
 

    GtkWidget *btn_encrypt = gtk_button_new_with_label("Encrypt");
    g_signal_connect(btn_encrypt, "clicked", G_CALLBACK(on_encrypt_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), btn_encrypt, 0, 3, 1, 1);

    GtkWidget *btn_decrypt = gtk_button_new_with_label("Decrypt");
    g_signal_connect(btn_decrypt, "clicked", G_CALLBACK(on_decrypt_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), btn_decrypt, 1, 3, 1, 1);

    label_status = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), label_status, 0, 4, 2, 1);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
