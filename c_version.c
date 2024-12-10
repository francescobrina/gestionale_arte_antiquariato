#include <gtk/gtk.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// Percorso del database SQLite
#define DB_PATH "magazzino_arte.db"

// Struttura per tenere traccia dei widget e della connessione al DB
typedef struct {
    GtkWidget *window;
    GtkWidget *treeview;
    GtkWidget *btn_aggiungi;
    GtkWidget *btn_vendi;
    GtkWidget *btn_elimina;
    GtkWidget *btn_aggiorna;
    sqlite3 *db;
} AppData;

// Prototipi delle funzioni
static void crea_tabelle(sqlite3 *db);
static int connetti_db(sqlite3 **db);
static void carica_dati(AppData *app);
static void on_btn_aggiorna_clicked(GtkButton *button, AppData *app);
static void on_btn_elimina_clicked(GtkButton *button, AppData *app);
static void on_btn_aggiungi_clicked(GtkButton *button, AppData *app);
static void on_btn_vendi_clicked(GtkButton *button, AppData *app);

// Funzioni di supporto per dialoghi
static GtkWidget* create_labeled_entry(const char *label_text, GtkWidget *grid, int row);

// Callbacks per finestre di dialogo
static void on_btn_salva_articolo_clicked(GtkButton *button, gpointer user_data);
static void on_btn_registra_vendita_clicked(GtkButton *button, gpointer user_data);

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppData app;
    memset(&app, 0, sizeof(AppData));

    // Connessione al DB
    if (access(DB_PATH, F_OK) != 0) {
        // Il file non esiste, verrà creato
        if (connetti_db(&app.db) == SQLITE_OK) {
            crea_tabelle(app.db);
            sqlite3_close(app.db);
        }
    }

    // Riapriamo la connessione per utilizzo generale
    if (connetti_db(&app.db) != SQLITE_OK) {
        fprintf(stderr, "Impossibile connettersi al database.\n");
        return 1;
    }

    // Creazione della finestra principale
    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), "Gestionale Magazzino Arte e Antiquariato");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 1000, 600);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app.window), vbox);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    app.btn_aggiungi = gtk_button_new_with_label("Aggiungi Articolo");
    gtk_box_pack_start(GTK_BOX(hbox), app.btn_aggiungi, FALSE, FALSE, 5);
    g_signal_connect(app.btn_aggiungi, "clicked", G_CALLBACK(on_btn_aggiungi_clicked), &app);

    app.btn_vendi = gtk_button_new_with_label("Registra Vendita");
    gtk_box_pack_start(GTK_BOX(hbox), app.btn_vendi, FALSE, FALSE, 5);
    g_signal_connect(app.btn_vendi, "clicked", G_CALLBACK(on_btn_vendi_clicked), &app);

    app.btn_elimina = gtk_button_new_with_label("Elimina Articolo");
    gtk_box_pack_start(GTK_BOX(hbox), app.btn_elimina, FALSE, FALSE, 5);
    g_signal_connect(app.btn_elimina, "clicked", G_CALLBACK(on_btn_elimina_clicked), &app);

    app.btn_aggiorna = gtk_button_new_with_label("Aggiorna Lista");
    gtk_box_pack_start(GTK_BOX(hbox), app.btn_aggiorna, FALSE, FALSE, 5);
    g_signal_connect(app.btn_aggiorna, "clicked", G_CALLBACK(on_btn_aggiorna_clicked), &app);

    // Creazione del TreeView
    app.treeview = gtk_tree_view_new();
    gtk_box_pack_start(GTK_BOX(vbox), app.treeview, TRUE, TRUE, 5);

    // Aggiunta delle colonne
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *col;

    const char *columns[] = {"ID", "Nome", "Artista", "Periodo", "Misure", "Quantità", "Prezzo Acquisto", "Stato"};
    int i;
    for (i = 0; i < 8; i++) {
        col = gtk_tree_view_column_new_with_attributes(columns[i], renderer, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(app.treeview), col);
    }

    // Creazione del model
    GtkListStore *store = gtk_list_store_new(8, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(app.treeview), GTK_TREE_MODEL(store));
    g_object_unref(store);

    carica_dati(&app);

    gtk_widget_show_all(app.window);
    gtk_main();

    sqlite3_close(app.db);
    return 0;
}

// Funzione per connettersi al DB
static int connetti_db(sqlite3 **db) {
    int rc = sqlite3_open(DB_PATH, db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Errore di connessione al DB: %s\n", sqlite3_errmsg(*db));
        return rc;
    }
    return SQLITE_OK;
}

// Creazione delle tabelle
static void crea_tabelle(sqlite3 *db) {
    const char *sql_articoli =
        "CREATE TABLE IF NOT EXISTS articoli ("
        "articolo_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nome TEXT NOT NULL,"
        "descrizione TEXT,"
        "artista TEXT,"
        "periodo TEXT,"
        "misure TEXT,"
        "data_acquisizione DATE,"
        "prezzo_acquisto REAL,"
        "quantita INTEGER DEFAULT 1"
        ");";

    const char *sql_vendite =
        "CREATE TABLE IF NOT EXISTS vendite ("
        "vendita_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "articolo_id INTEGER,"
        "data_vendita DATE,"
        "prezzo_vendita REAL,"
        "nome_cliente TEXT,"
        "FOREIGN KEY (articolo_id) REFERENCES articoli(articolo_id)"
        ");";

    char *err_msg = NULL;
    if (sqlite3_exec(db, sql_articoli, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Errore creazione tabella articoli: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    if (sqlite3_exec(db, sql_vendite, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Errore creazione tabella vendite: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

// Carica i dati nel TreeView
static void carica_dati(AppData *app) {
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(app->treeview)));
    gtk_list_store_clear(store);

    const char *sql =
        "SELECT articolo_id, nome, artista, periodo, misure, quantita, prezzo_acquisto, "
        "CASE WHEN quantita = 0 THEN 'Venduto' ELSE 'Disponibile' END AS stato "
        "FROM articoli ORDER BY stato ASC;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(app->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Errore query: %s\n", sqlite3_errmsg(app->db));
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int articolo_id = sqlite3_column_int(stmt, 0);
        const char *nome = (const char*)sqlite3_column_text(stmt, 1);
        const char *artista = (const char*)sqlite3_column_text(stmt, 2);
        const char *periodo = (const char*)sqlite3_column_text(stmt, 3);
        const char *misure = (const char*)sqlite3_column_text(stmt, 4);
        int quantita = sqlite3_column_int(stmt, 5);
        double prezzo_acq = sqlite3_column_double(stmt, 6);
        const char *stato = (const char*)sqlite3_column_text(stmt, 7);

        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, articolo_id,
                           1, nome ? nome : "",
                           2, artista ? artista : "",
                           3, periodo ? periodo : "",
                           4, misure ? misure : "",
                           5, quantita,
                           6, prezzo_acq,
                           7, stato ? stato : "",
                           -1);
    }

    sqlite3_finalize(stmt);
}

// Callback per aggiornare la lista
static void on_btn_aggiorna_clicked(GtkButton *button, AppData *app) {
    carica_dati(app);
}

// Callback per eliminare un articolo
static void on_btn_elimina_clicked(GtkButton *button, AppData *app) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(app->treeview));
    GtkTreeModel *model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        int articolo_id;
        gtk_tree_model_get(model, &iter, 0, &articolo_id, -1);

        // Chiede conferma
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_QUESTION,
                                                   GTK_BUTTONS_YES_NO,
                                                   "Sei sicuro di voler eliminare l'articolo ID %d?", articolo_id);
        int response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (response == GTK_RESPONSE_YES) {
            char sql[256];
            snprintf(sql, sizeof(sql), "DELETE FROM articoli WHERE articolo_id = %d;", articolo_id);
            char *err_msg = NULL;
            if (sqlite3_exec(app->db, sql, 0, 0, &err_msg) != SQLITE_OK) {
                GtkWidget *err_dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                               GTK_DIALOG_MODAL,
                                                               GTK_MESSAGE_ERROR,
                                                               GTK_BUTTONS_OK,
                                                               "Errore durante l'eliminazione: %s", err_msg);
                gtk_dialog_run(GTK_DIALOG(err_dialog));
                gtk_widget_destroy(err_dialog);
                sqlite3_free(err_msg);
            } else {
                GtkWidget *info = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                         GTK_DIALOG_MODAL,
                                                         GTK_MESSAGE_INFO,
                                                         GTK_BUTTONS_OK,
                                                         "Articolo eliminato con successo.");
                gtk_dialog_run(GTK_DIALOG(info));
                gtk_widget_destroy(info);
                carica_dati(app);
            }
        }
    } else {
        GtkWidget *warn = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_WARNING,
                                                 GTK_BUTTONS_OK,
                                                 "Seleziona un articolo da eliminare.");
        gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);
    }
}

// Finestra per aggiungere articolo
static void on_btn_aggiungi_clicked(GtkButton *button, AppData *app) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Aggiungi Articolo",
                                                    GTK_WINDOW(app->window),
                                                    GTK_DIALOG_MODAL,
                                                    "Salva", GTK_RESPONSE_OK,
                                                    "Annulla", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content), grid);

    GtkWidget *entry_nome = create_labeled_entry("Nome*", grid, 0);
    GtkWidget *entry_descrizione = create_labeled_entry("Descrizione", grid, 1);
    GtkWidget *entry_artista = create_labeled_entry("Artista", grid, 2);
    GtkWidget *entry_periodo = create_labeled_entry("Periodo", grid, 3);
    GtkWidget *entry_misure = create_labeled_entry("Misure", grid, 4);
    GtkWidget *entry_data = create_labeled_entry("Data Acquisizione (YYYY-MM-DD)", grid, 5);
    GtkWidget *entry_prezzo = create_labeled_entry("Prezzo Acquisto*", grid, 6);
    GtkWidget *entry_quantita = create_labeled_entry("Quantità*", grid, 7);

    // Mostra il dialogo
    gtk_widget_show_all(dialog);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        const char *nome = gtk_entry_get_text(GTK_ENTRY(entry_nome));
        const char *descrizione = gtk_entry_get_text(GTK_ENTRY(entry_descrizione));
        const char *artista = gtk_entry_get_text(GTK_ENTRY(entry_artista));
        const char *periodo = gtk_entry_get_text(GTK_ENTRY(entry_periodo));
        const char *misure = gtk_entry_get_text(GTK_ENTRY(entry_misure));
        const char *data_aq = gtk_entry_get_text(GTK_ENTRY(entry_data));
        const char *prezzo = gtk_entry_get_text(GTK_ENTRY(entry_prezzo));
        const char *quantita = gtk_entry_get_text(GTK_ENTRY(entry_quantita));

        if (strlen(nome) == 0 || strlen(prezzo) == 0 || strlen(quantita) == 0) {
            GtkWidget *warn = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_WARNING,
                                                     GTK_BUTTONS_OK,
                                                     "Campi obbligatori mancanti.");
            gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);
        } else {
            const char *sql = "INSERT INTO articoli (nome, descrizione, artista, periodo, misure, data_acquisizione, prezzo_acquisto, quantita) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(app->db, sql, -1, &stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, descrizione, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, artista, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 4, periodo, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 5, misure, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 6, data_aq, -1, SQLITE_TRANSIENT);
                sqlite3_bind_double(stmt, 7, atof(prezzo));
                sqlite3_bind_int(stmt, 8, atoi(quantita));

                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    GtkWidget *info = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                             GTK_DIALOG_MODAL,
                                                             GTK_MESSAGE_INFO,
                                                             GTK_BUTTONS_OK,
                                                             "Articolo aggiunto con successo.");
                    gtk_dialog_run(GTK_DIALOG(info));
                    gtk_widget_destroy(info);
                    carica_dati(app);
                } else {
                    GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                            GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_ERROR,
                                                            GTK_BUTTONS_OK,
                                                            "Errore durante l'inserimento: %s",
                                                            sqlite3_errmsg(app->db));
                    gtk_dialog_run(GTK_DIALOG(err));
                    gtk_widget_destroy(err);
                }
                sqlite3_finalize(stmt);
            }
        }
    }

    gtk_widget_destroy(dialog);
}

// Finestra per registrare una vendita
static void on_btn_vendi_clicked(GtkButton *button, AppData *app) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Registra Vendita",
                                                    GTK_WINDOW(app->window),
                                                    GTK_DIALOG_MODAL,
                                                    "Vendi", GTK_RESPONSE_OK,
                                                    "Annulla", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content), grid);

    GtkWidget *entry_id = create_labeled_entry("ID Articolo*", grid, 0);
    GtkWidget *entry_cliente = create_labeled_entry("Nome Cliente", grid, 1);
    GtkWidget *entry_prezzo_v = create_labeled_entry("Prezzo di Vendita*", grid, 2);

    gtk_widget_show_all(dialog);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        const char *id_str = gtk_entry_get_text(GTK_ENTRY(entry_id));
        const char *cliente = gtk_entry_get_text(GTK_ENTRY(entry_cliente));
        const char *prezzo_str = gtk_entry_get_text(GTK_ENTRY(entry_prezzo_v));
        if (strlen(id_str) == 0 || strlen(prezzo_str) == 0) {
            GtkWidget *warn = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_WARNING,
                                                     GTK_BUTTONS_OK,
                                                     "ID Articolo e Prezzo Vendita sono obbligatori.");
            gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);
        } else {
            int articolo_id = atoi(id_str);

            // Verifica quantità
            const char *sql_check = "SELECT quantita FROM articoli WHERE articolo_id = ?;";
            sqlite3_stmt *stmt_check;
            if (sqlite3_prepare_v2(app->db, sql_check, -1, &stmt_check, NULL) == SQLITE_OK) {
                sqlite3_bind_int(stmt_check, 1, articolo_id);
                int step = sqlite3_step(stmt_check);
                if (step == SQLITE_ROW) {
                    int quantita = sqlite3_column_int(stmt_check, 0);
                    sqlite3_finalize(stmt_check);

                    if (quantita > 0) {
                        // Aggiorna quantità
                        const char *sql_upd = "UPDATE articoli SET quantita = quantita - 1 WHERE articolo_id = ?;";
                        sqlite3_stmt *stmt_upd;
                        if (sqlite3_prepare_v2(app->db, sql_upd, -1, &stmt_upd, NULL) == SQLITE_OK) {
                            sqlite3_bind_int(stmt_upd, 1, articolo_id);
                            sqlite3_step(stmt_upd);
                            sqlite3_finalize(stmt_upd);
                        }

                        // Inserimento vendita
                        const char *sql_vendita = "INSERT INTO vendite (articolo_id, data_vendita, prezzo_vendita, nome_cliente) VALUES (?, ?, ?, ?);";
                        sqlite3_stmt *stmt_v;
                        if (sqlite3_prepare_v2(app->db, sql_vendita, -1, &stmt_v, NULL) == SQLITE_OK) {
                            // Data corrente
                            time_t t = time(NULL);
                            struct tm *tm_info = localtime(&t);
                            char data_str[11];
                            strftime(data_str, 11, "%Y-%m-%d", tm_info);

                            sqlite3_bind_int(stmt_v, 1, articolo_id);
                            sqlite3_bind_text(stmt_v, 2, data_str, -1, SQLITE_TRANSIENT);
                            sqlite3_bind_double(stmt_v, 3, atof(prezzo_str));
                            sqlite3_bind_text(stmt_v, 4, cliente, -1, SQLITE_TRANSIENT);

                            if (sqlite3_step(stmt_v) == SQLITE_DONE) {
                                GtkWidget *info = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                                         GTK_DIALOG_MODAL,
                                                           