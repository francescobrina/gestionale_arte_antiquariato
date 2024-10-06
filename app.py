import tkinter as tk
from tkinter import messagebox, ttk
import sqlite3
from datetime import datetime
import os

# Percorso del database SQLite
DB_PATH = 'magazzino_arte.db'

# Funzione per connettersi al database SQLite
def connetti_db():
    try:
        conn = sqlite3.connect(DB_PATH)
        return conn
    except sqlite3.Error as err:
        messagebox.showerror("Errore Database", f"Errore di connessione: {err}")
        return None

# Funzione per creare le tabelle se non esistono
def crea_tabelle():
    conn = connetti_db()
    if conn:
        cursor = conn.cursor()
        # Creazione della tabella articoli
        cursor.execute("""
        CREATE TABLE IF NOT EXISTS articoli (
            articolo_id INTEGER PRIMARY KEY AUTOINCREMENT,
            nome TEXT NOT NULL,
            descrizione TEXT,
            artista TEXT,
            periodo TEXT,
            misure TEXT,
            data_acquisizione DATE,
            prezzo_acquisto REAL,
            quantita INTEGER DEFAULT 1
        )
        """)
        # Creazione della tabella vendite
        cursor.execute("""
        CREATE TABLE IF NOT EXISTS vendite (
            vendita_id INTEGER PRIMARY KEY AUTOINCREMENT,
            articolo_id INTEGER,
            data_vendita DATE,
            prezzo_vendita REAL,
            nome_cliente TEXT,
            FOREIGN KEY (articolo_id) REFERENCES articoli(articolo_id)
        )
        """)
        conn.commit()
        cursor.close()
        conn.close()

# Classe principale dell'applicazione
class GestionaleMagazzino:
    def __init__(self, master):
        self.master = master
        master.title("Gestionale Magazzino Arte e Antiquariato")
        master.geometry("1000x600")

        # Creazione del frame principale
        main_frame = tk.Frame(master)
        main_frame.pack(fill=tk.BOTH, expand=True)

        # Creazione dei pulsanti principali
        btn_frame = tk.Frame(main_frame)
        btn_frame.pack(side=tk.TOP, fill=tk.X)

        self.btn_aggiungi = tk.Button(btn_frame, text="Aggiungi Articolo", width=20, command=self.aggiungi_articolo)
        self.btn_aggiungi.pack(side=tk.LEFT, padx=5, pady=10)

        self.btn_vendi = tk.Button(btn_frame, text="Registra Vendita", width=20, command=self.registra_vendita)
        self.btn_vendi.pack(side=tk.LEFT, padx=5, pady=10)

        self.btn_elimina = tk.Button(btn_frame, text="Elimina Articolo", width=20, command=self.elimina_articolo)
        self.btn_elimina.pack(side=tk.LEFT, padx=5, pady=10)

        self.btn_aggiorna = tk.Button(btn_frame, text="Aggiorna Lista", width=20, command=self.carica_dati)
        self.btn_aggiorna.pack(side=tk.LEFT, padx=5, pady=10)

        # Creazione della tabella del magazzino
        self.columns = ("ID", "Nome", "Artista", "Periodo", "Misure", "Quantità", "Prezzo Acquisto", "Stato")
        self.tree = ttk.Treeview(main_frame, columns=self.columns, show='headings')

        for col in self.columns:
            self.tree.heading(col, text=col)
            self.tree.column(col, minwidth=0, width=120)

        # Aggiunta dello stile per i prodotti venduti
        self.tree.tag_configure('venduto', foreground='gray')

        self.tree.pack(fill=tk.BOTH, expand=True)

        # Caricamento iniziale dei dati
        self.carica_dati()

    # Funzione per caricare i dati nella tabella
    def carica_dati(self):
        # Pulizia della tabella
        for item in self.tree.get_children():
            self.tree.delete(item)

        conn = connetti_db()
        if conn:
            cursor = conn.cursor()
            query = """
            SELECT articolo_id, nome, artista, periodo, misure, quantita, prezzo_acquisto,
            CASE WHEN quantita = 0 THEN 'Venduto' ELSE 'Disponibile' END AS stato
            FROM articoli
            ORDER BY stato ASC
            """
            try:
                cursor.execute(query)
                for row in cursor.fetchall():
                    articolo_id, nome, artista, periodo, misure, quantita, prezzo_acquisto, stato = row
                    values = (articolo_id, nome, artista, periodo, misure, quantita, prezzo_acquisto, stato)
                    if stato == 'Venduto':
                        self.tree.insert("", tk.END, values=values, tags=('venduto',))
                    else:
                        self.tree.insert("", tk.END, values=values)
            except sqlite3.Error as err:
                messagebox.showerror("Errore Database", f"Errore durante la lettura: {err}")
            finally:
                cursor.close()
                conn.close()

    # Funzione per aggiungere un nuovo articolo
    def aggiungi_articolo(self):
        def salva_articolo():
            nome = entry_nome.get()
            descrizione = entry_descrizione.get()
            artista = entry_artista.get()
            periodo = entry_periodo.get()
            misure = entry_misure.get()
            data_acquisizione = entry_data_acquisizione.get()
            prezzo_acquisto = entry_prezzo_acquisto.get()
            quantita = entry_quantita.get()

            if not nome or not prezzo_acquisto or not quantita:
                messagebox.showwarning("Campi obbligatori", "Per favore, compila tutti i campi obbligatori.")
                return

            conn = connetti_db()
            if conn:
                cursor = conn.cursor()
                query = ("INSERT INTO articoli "
                         "(nome, descrizione, artista, periodo, misure, data_acquisizione, prezzo_acquisto, quantita) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?)")
                dati = (nome, descrizione, artista, periodo, misure, data_acquisizione, prezzo_acquisto, quantita)
                try:
                    cursor.execute(query, dati)
                    conn.commit()
                    messagebox.showinfo("Successo", "Articolo aggiunto con successo.")
                    finestra_aggiungi.destroy()
                    self.carica_dati()
                except sqlite3.Error as err:
                    messagebox.showerror("Errore Database", f"Errore durante l'inserimento: {err}")
                finally:
                    cursor.close()
                    conn.close()

        finestra_aggiungi = tk.Toplevel(self.master)
        finestra_aggiungi.title("Aggiungi Articolo")

        tk.Label(finestra_aggiungi, text="Nome*:").grid(row=0, column=0, pady=5)
        entry_nome = tk.Entry(finestra_aggiungi)
        entry_nome.grid(row=0, column=1, pady=5)

        tk.Label(finestra_aggiungi, text="Descrizione:").grid(row=1, column=0, pady=5)
        entry_descrizione = tk.Entry(finestra_aggiungi)
        entry_descrizione.grid(row=1, column=1, pady=5)

        tk.Label(finestra_aggiungi, text="Artista:").grid(row=2, column=0, pady=5)
        entry_artista = tk.Entry(finestra_aggiungi)
        entry_artista.grid(row=2, column=1, pady=5)

        tk.Label(finestra_aggiungi, text="Periodo:").grid(row=3, column=0, pady=5)
        entry_periodo = tk.Entry(finestra_aggiungi)
        entry_periodo.grid(row=3, column=1, pady=5)

        tk.Label(finestra_aggiungi, text="Misure:").grid(row=4, column=0, pady=5)
        entry_misure = tk.Entry(finestra_aggiungi)
        entry_misure.grid(row=4, column=1, pady=5)

        tk.Label(finestra_aggiungi, text="Data Acquisizione (YYYY-MM-DD):").grid(row=5, column=0, pady=5)
        entry_data_acquisizione = tk.Entry(finestra_aggiungi)
        entry_data_acquisizione.grid(row=5, column=1, pady=5)

        tk.Label(finestra_aggiungi, text="Prezzo Acquisto*:").grid(row=6, column=0, pady=5)
        entry_prezzo_acquisto = tk.Entry(finestra_aggiungi)
        entry_prezzo_acquisto.grid(row=6, column=1, pady=5)

        tk.Label(finestra_aggiungi, text="Quantità*:").grid(row=7, column=0, pady=5)
        entry_quantita = tk.Entry(finestra_aggiungi)
        entry_quantita.grid(row=7, column=1, pady=5)

        tk.Label(finestra_aggiungi, text="* Campi obbligatori").grid(row=8, column=0, columnspan=2)

        btn_salva = tk.Button(finestra_aggiungi, text="Salva", command=salva_articolo)
        btn_salva.grid(row=9, column=0, columnspan=2, pady=10)

    # Funzione per registrare una vendita
    def registra_vendita(self):
        def seleziona_articolo():
            articolo_id = entry_articolo_id.get()
            nome_cliente = entry_nome_cliente.get()
            prezzo_vendita = entry_prezzo_vendita.get()
            data_vendita = datetime.now().strftime('%Y-%m-%d')

            if not articolo_id or not prezzo_vendita:
                messagebox.showwarning("Campi obbligatori", "Per favore, compila tutti i campi obbligatori.")
                return

            conn = connetti_db()
            if conn:
                cursor = conn.cursor()
                # Verifica se l'articolo esiste e ha quantità sufficiente
                query_articolo = "SELECT quantita FROM articoli WHERE articolo_id = ?"
                cursor.execute(query_articolo, (articolo_id,))
                risultato = cursor.fetchone()
                if risultato:
                    quantita_disponibile = risultato[0]
                    if quantita_disponibile > 0:
                        # Aggiorna la quantità dell'articolo
                        nuova_quantita = quantita_disponibile - 1
                        query_update = "UPDATE articoli SET quantita = ? WHERE articolo_id = ?"
                        cursor.execute(query_update, (nuova_quantita, articolo_id))
                        # Registra la vendita
                        query_vendita = ("INSERT INTO vendite (articolo_id, data_vendita, prezzo_vendita, nome_cliente) "
                                         "VALUES (?, ?, ?, ?)")
                        cursor.execute(query_vendita, (articolo_id, data_vendita, prezzo_vendita, nome_cliente))
                        conn.commit()
                        messagebox.showinfo("Successo", "Vendita registrata con successo.")
                        finestra_vendita.destroy()
                        self.carica_dati()
                    else:
                        messagebox.showwarning("Quantità Insufficiente", "L'articolo non è disponibile in magazzino.")
                else:
                    messagebox.showerror("Errore", "Articolo non trovato.")
                cursor.close()
                conn.close()

        finestra_vendita = tk.Toplevel(self.master)
        finestra_vendita.title("Registra Vendita")

        tk.Label(finestra_vendita, text="ID Articolo*:").grid(row=0, column=0, pady=5)
        entry_articolo_id = tk.Entry(finestra_vendita)
        entry_articolo_id.grid(row=0, column=1, pady=5)

        tk.Label(finestra_vendita, text="Nome Cliente:").grid(row=1, column=0, pady=5)
        entry_nome_cliente = tk.Entry(finestra_vendita)
        entry_nome_cliente.grid(row=1, column=1, pady=5)

        tk.Label(finestra_vendita, text="Prezzo di Vendita*:").grid(row=2, column=0, pady=5)
        entry_prezzo_vendita = tk.Entry(finestra_vendita)
        entry_prezzo_vendita.grid(row=2, column=1, pady=5)

        tk.Label(finestra_vendita, text="* Campi obbligatori").grid(row=3, column=0, columnspan=2)

        btn_vendi = tk.Button(finestra_vendita, text="Registra Vendita", command=seleziona_articolo)
        btn_vendi.grid(row=4, column=0, columnspan=2, pady=10)

    # Funzione per eliminare un articolo
    def elimina_articolo(self):
        selected_item = self.tree.focus()
        if not selected_item:
            messagebox.showwarning("Nessuna selezione", "Per favore, seleziona un articolo da eliminare.")
            return

        values = self.tree.item(selected_item, 'values')
        articolo_id = values[0]

        risposta = messagebox.askyesno("Conferma Eliminazione", f"Sei sicuro di voler eliminare l'articolo ID {articolo_id}?")
        if risposta:
            conn = connetti_db()
            if conn:
                cursor = conn.cursor()
                try:
                    # Elimina l'articolo dal database
                    query_elimina = "DELETE FROM articoli WHERE articolo_id = ?"
                    cursor.execute(query_elimina, (articolo_id,))
                    conn.commit()
                    messagebox.showinfo("Successo", "Articolo eliminato con successo.")
                    self.carica_dati()
                except sqlite3.Error as err:
                    messagebox.showerror("Errore Database", f"Errore durante l'eliminazione: {err}")
                finally:
                    cursor.close()
                    conn.close()

# Avvio dell'applicazione
if __name__ == "__main__":
    # Crea le tabelle al primo avvio se necessario
    if not os.path.exists(DB_PATH):
        crea_tabelle()
    else:
        # Controlla se le tabelle esistono
        conn = connetti_db()
        if conn:
            cursor = conn.cursor()
            cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='articoli'")
            if not cursor.fetchone():
                crea_tabelle()
            cursor.close()
            conn.close()

    root = tk.Tk()
    app = GestionaleMagazzino(root)
    root.mainloop()