# Gestionale Magazzino Arte e Antiquariato

Un'applicazione Python per gestire il magazzino e le vendite di beni d'arte e antiquariato, utilizzando Tkinter per l'interfaccia grafica e SQLite come database locale.

## Descrizione

Il **Gestionale Magazzino Arte e Antiquariato** è un'applicazione desktop progettata per aiutare gli operatori nel settore dell'arte e dell'antiquariato a gestire efficacemente il loro inventario e le vendite. L'applicazione permette di:

- Aggiungere nuovi articoli al magazzino con dettagli come nome, descrizione, artista, periodo, misure, data di acquisizione, prezzo di acquisto e quantità.
- Visualizzare tutti gli articoli presenti nel magazzino, con la possibilità di distinguere tra articoli disponibili e venduti.
- Registrare le vendite degli articoli, specificando il prezzo di vendita e il nome del cliente.
- Eliminare articoli dal magazzino quando necessario.

L'applicazione utilizza SQLite per memorizzare i dati localmente, rendendo semplice l'installazione e l'utilizzo senza la necessità di un database server esterno.

## Caratteristiche Principali

- **Interfaccia Utente Intuitiva**: Grazie a Tkinter, l'applicazione offre un'interfaccia grafica semplice e intuitiva.
- **Database Locale con SQLite**: I dati vengono memorizzati in un database SQLite locale, facilitando la gestione e la portabilità.
- **Gestione Completa del Magazzino**:
  - Aggiunta di nuovi articoli con informazioni dettagliate.
  - Visualizzazione dell'inventario con stato degli articoli (Disponibile o Venduto).
  - Registrazione delle vendite con aggiornamento automatico delle quantità.
  - Eliminazione di articoli dal magazzino.
- **Persistenza dei Dati**: Tutte le informazioni inserite vengono salvate e mantenute tra le diverse esecuzioni dell'applicazione.

## Requisiti

- **Python 3.x**: Assicurati di avere Python 3 installato sul tuo sistema.
- **Tkinter**: Dovrebbe essere incluso nella tua installazione di Python.
- **sqlite3**: Modulo standard incluso in Python per interagire con database SQLite.

## Installazione

1. **Clona il Repository o Scarica il Codice Sorgente**:

   ```bash
   git clone https://github.com/tuo_username/gestionale-magazzino-arte.git
   ```

2. **Naviga nella Cartella del Progetto**:

   ```bash
   cd gestionale-magazzino-arte
   ```

3. **(Opzionale) Crea un Ambiente Virtuale**:

   È consigliabile utilizzare un ambiente virtuale per isolare le dipendenze del progetto.

   ```bash
   python -m venv venv
   source venv/bin/activate  # Su Windows: venv\Scripts\activate
   ```

4. **Installa le Dipendenze**:

   Non sono necessarie dipendenze esterne oltre ai moduli standard di Python.

## Esecuzione dell'Applicazione

Avvia l'applicazione eseguendo il file Python principale:

```bash
python gestionale_magazzino.py
```

## Utilizzo dell'Applicazione

### Aggiungere un Articolo

1. Clicca sul pulsante **"Aggiungi Articolo"**.
2. Compila i campi richiesti nel modulo che appare:
   - **Nome***: Nome dell'articolo (campo obbligatorio).
   - **Descrizione**: Descrizione dettagliata dell'articolo.
   - **Artista**: Nome dell'artista o creatore.
   - **Periodo**: Periodo storico o datazione dell'articolo.
   - **Misure**: Dimensioni dell'articolo (es. "30x40 cm").
   - **Data Acquisizione**: Data di acquisizione nel formato `YYYY-MM-DD`.
   - **Prezzo Acquisto***: Prezzo di acquisto dell'articolo (campo obbligatorio).
   - **Quantità***: Quantità disponibile (campo obbligatorio).
3. Clicca su **"Salva"** per aggiungere l'articolo al magazzino.

### Visualizzare il Magazzino

- Gli articoli nel magazzino sono visualizzati nella tabella principale.
- Gli articoli **venduti** appaiono in grigio e sono contrassegnati come **"Venduto"** nella colonna "Stato".
- Gli articoli **disponibili** sono contrassegnati come **"Disponibile"**.

### Registrare una Vendita

1. Seleziona un articolo dalla tabella (opzionale).
2. Clicca sul pulsante **"Registra Vendita"**.
3. Inserisci le seguenti informazioni:
   - **ID Articolo***: ID dell'articolo da vendere (precompilato se hai selezionato un articolo).
   - **Nome Cliente**: Nome del cliente (opzionale).
   - **Prezzo di Vendita***: Prezzo al quale l'articolo viene venduto (campo obbligatorio).
4. Clicca su **"Registra Vendita"** per confermare.
5. La quantità dell'articolo verrà aggiornata. Se la quantità raggiunge zero, l'articolo sarà contrassegnato come venduto.

### Eliminare un Articolo

1. Seleziona l'articolo da eliminare nella tabella.
2. Clicca sul pulsante **"Elimina Articolo"**.
3. Conferma l'eliminazione quando richiesto.

### Aggiornare la Lista

- Clicca su **"Aggiorna Lista"** per ricaricare i dati dal database, utile se ci sono state modifiche esterne.

## Struttura del Database

Il database SQLite `magazzino_arte.db` viene creato automaticamente nella directory del progetto al primo avvio dell'applicazione.

### Tabelle Principali

- **articoli**:
  - `articolo_id`: ID univoco dell'articolo (PRIMARY KEY).
  - `nome`: Nome dell'articolo.
  - `descrizione`: Descrizione dell'articolo.
  - `artista`: Nome dell'artista o creatore.
  - `periodo`: Periodo storico o datazione.
  - `misure`: Dimensioni dell'articolo.
  - `data_acquisizione`: Data di acquisizione.
  - `prezzo_acquisto`: Prezzo di acquisto.
  - `quantita`: Quantità disponibile.

- **vendite**:
  - `vendita_id`: ID univoco della vendita (PRIMARY KEY).
  - `articolo_id`: ID dell'articolo venduto (FOREIGN KEY che riferisce `articoli(articolo_id)`).
  - `data_vendita`: Data della vendita.
  - `prezzo_vendita`: Prezzo di vendita.
  - `nome_cliente`: Nome del cliente.

## Personalizzazione e Estensioni

- **Modifica Articoli**: È possibile estendere l'applicazione per permettere la modifica dei dettagli di un articolo esistente.
- **Filtri e Ricerca**: Implementa funzioni di ricerca e filtri per facilitare la gestione di un inventario più grande.
- **Reportistica**: Genera report sulle vendite, sull'inventario, sui profitti, ecc.
- **Sicurezza**: Aggiungi meccanismi di autenticazione se necessario.

## Contribuire

Siamo aperti a contributi! Se vuoi migliorare questa applicazione:

1. Fai un fork del progetto.
2. Crea un nuovo branch per le tue modifiche:

   ```bash
   git checkout -b feature/nome-feature
   ```

3. Committa le tue modifiche:

   ```bash
   git commit -m "Aggiunta di una nuova funzionalità"
   ```

4. Fai push al branch:

   ```bash
   git push origin feature/nome-feature
   ```

5. Apri una Pull Request.

## Licenza

Questo progetto è distribuito sotto la licenza **MIT**. Consulta il file [LICENSE](LICENSE) per ulteriori informazioni.

## Contatti

Per domande o supporto, contattami [francescobrina9@gmail.com](malto:francescobrina9@gmail.com
).

---

Grazie per aver utilizzato il **Gestionale Magazzino Arte e Antiquariato**!
