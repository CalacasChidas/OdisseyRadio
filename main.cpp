#include <gtk/gtk.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <SDL2/SDL.h>
#include <SerialStream.h>
#include <thread>

struct Song {
    std::string title;
    std::string artist;
    std::string album;
    std::string audioFilePath;
};

int base = 0, end = 20;
GtkWidget *window, *PruebaArduino, *Exit, *TreeView, *Up, *Down, *Pag;
GtkLabel *memoryLabel;
GtkProgressBar *progressMemory;
GtkListStore *listStore;
std::vector<Song> library;
std::string csvFilePath = "/home/aleprominecraft/fma/data/fma_metadata/tracks.csv";
SDL_AudioDeviceID deviceId = 0;
bool isPlaying = false;

std::vector<Song> loadLibraryFromCSV(const std::string& csvFilePath, int start, int count) {
    std::vector<Song> library;
    // Abre el archivo CSV
    std::ifstream csvFile(csvFilePath);
    std::cout << "CSV file readed!\n";
    if (!csvFile.is_open()) {
        std::cerr << "Error al abrir el archivo CSV" << std::endl;
        return library;
    }
    std::string line;
    int currentLine = 0;
    while (std::getline(csvFile, line) && currentLine < start + count) {
        if (currentLine >= start) {
            std::istringstream iss(line);
            std::string title, artist, album, audioFilePath; // Agrega audioFilePath
            if (std::getline(iss, title, ',') &&
                std::getline(iss, artist, ',') &&
                std::getline(iss, album, ',') && // Supongo que la columna de audioFilePath es la cuarta (índice 3)
                std::getline(iss, audioFilePath)) { // Lee la ruta del archivo de sonido
                library.push_back({title, artist, album, audioFilePath}); // Agrega audioFilePath a la estructura Song
            }
        }
        currentLine++;
    }
    // Cierra el archivo CSV
    csvFile.close();
    return library;
}

void pruebaArduino(){
    std::cout << "Stop button pressed\n";
    if (!isPlaying) {
        SDL_PauseAudioDevice(deviceId, 0); // Reproducir audio
        isPlaying = true;
    } else {
        SDL_PauseAudioDevice(deviceId, 1); // Pausar audio
        isPlaying = false;
    }
}

void on_row_activated(GtkTreeView* treeview, GtkTreePath* path, GtkTreeViewColumn* column, gpointer user_data) {
    GtkTreeModel* model;
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(listStore), &iter, path)) {
        // Obtiene la ruta del archivo de sonido de la canción seleccionada
        std::string audioFilePath;
        gtk_tree_model_get(GTK_TREE_MODEL(listStore), &iter, 3, &audioFilePath, -1);

        SDL_Init(SDL_INIT_AUDIO);
        SDL_AudioSpec wavSpec;
        Uint32 wavLength;
        Uint8* wavBuffer;

        if (SDL_LoadWAV(audioFilePath.c_str(), &wavSpec, &wavBuffer, &wavLength) == NULL) {
            std::cerr << "Error al cargar el archivo de sonido: " << SDL_GetError() << std::endl;
            return;
        }

        SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
        if (deviceId == 0) {
            std::cerr << "Error al abrir el dispositivo de audio: " << SDL_GetError() << std::endl;
            SDL_FreeWAV(wavBuffer);
            return;
        }

        int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
        if (success < 0) {
            std::cerr << "Error al encolar el audio: " << SDL_GetError() << std::endl;
            SDL_CloseAudioDevice(deviceId);
            SDL_FreeWAV(wavBuffer);
            return;
        }

        SDL_PauseAudioDevice(deviceId, 0); // Reproducir audio

        // Esperar hasta que el audio se reproduzca completamente
        while (SDL_GetQueuedAudioSize(deviceId) > 0) {
            SDL_Delay(100);
        }

        SDL_CloseAudioDevice(deviceId);
        SDL_FreeWAV(wavBuffer);
        SDL_Quit();
    }
}

double getMemoryUsage(){
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    double total_mem = 0.0;
    double available_mem = 0.0;
    while (std::getline(meminfo, line)) {
        std::istringstream iss(line);
        std::string key;
        double value;
        if (iss >> key >> value) {
            if (key == "MemTotal:") {
                total_mem = value;
            } else if (key == "MemAvailable:") {
                available_mem = value;
            }
        }
    }
    if (total_mem == 0.0 || available_mem == 0.0) {
        // No se pudo obtener la información necesaria
        return -1.0;
    }
    // Calcula el porcentaje de memoria usada
    double used_mem = total_mem - available_mem;
    double usage_percentage = (used_mem / total_mem) * 100.0;
    return usage_percentage;
}

gboolean updateLabel(GtkLabel *label) {
    double memory_usage = getMemoryUsage();
    if (memory_usage >= 0.0) {
        // Formatea el porcentaje de memoria como una cadena con 2 decimales
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << memory_usage;
        std::string memory_usage_str = ss.str() + "%";

        // Actualiza el label
        gtk_label_set_text(label, memory_usage_str.c_str());
        gtk_progress_bar_set_fraction(progressMemory, memory_usage / 100.0);
    } else {
        // No se pudo obtener la información de memoria
        gtk_label_set_text(label, "Error");
    }
    // Devuelve TRUE para que el temporizador siga ejecutándose
    return TRUE;
}

void exit_app(){
    std::cout << "Closing app!\n";
    gtk_main_quit();
}

void loadMoreSongs(GtkListStore* listStore, const std::vector<Song>& library, int start, int count) {
    for (int i = start; i < start + count && i < library.size(); ++i) {
        GtkTreeIter iter;
        gtk_list_store_append(listStore, &iter);
        gtk_list_store_set(listStore, &iter, 0, library[i].title.c_str(), 1, library[i].artist.c_str(), 2, library[i].album.c_str(), -1);
    }
}

void subirLista(){
    if (base > 0){
        base-=20;
        end -= 20;
        library = loadLibraryFromCSV(csvFilePath,base,end);
        gtk_tree_view_set_model(GTK_TREE_VIEW(TreeView), GTK_TREE_MODEL(listStore));
        GtkTreeViewColumn* titleColumn = gtk_tree_view_column_new_with_attributes("Título", gtk_cell_renderer_text_new(), "text", 0, NULL);
        GtkTreeViewColumn* artistColumn = gtk_tree_view_column_new_with_attributes("Artista", gtk_cell_renderer_text_new(), "text", 1, NULL);
        GtkTreeViewColumn* albumColumn = gtk_tree_view_column_new_with_attributes("Álbum", gtk_cell_renderer_text_new(), "text", 2, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), titleColumn);
        gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), artistColumn);
        gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), albumColumn);
        gtk_tree_view_column_set_fixed_width(albumColumn, 140);
    }else{
    }
}

void bajarLista(){
    if (end <= 20){
        base+=20;
        end += 20;
        library = loadLibraryFromCSV(csvFilePath,base,end);
        gtk_tree_view_set_model(GTK_TREE_VIEW(TreeView), GTK_TREE_MODEL(listStore));
        GtkTreeViewColumn* titleColumn = gtk_tree_view_column_new_with_attributes("Título", gtk_cell_renderer_text_new(), "text", 0, NULL);
        GtkTreeViewColumn* artistColumn = gtk_tree_view_column_new_with_attributes("Artista", gtk_cell_renderer_text_new(), "text", 1, NULL);
        GtkTreeViewColumn* albumColumn = gtk_tree_view_column_new_with_attributes("Álbum", gtk_cell_renderer_text_new(), "text", 2, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), titleColumn);
        gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), artistColumn);
        gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), albumColumn);
        gtk_tree_view_column_set_fixed_width(albumColumn, 140);
    }else{
    }
}

void paginateLibrary(int currentPage) {
    // Calcula el índice de inicio y fin de la página actual
    int start = currentPage * 3;
    int end = start + 3;

    // Borra todas las filas existentes en el TreeView
    gtk_list_store_clear(listStore);

    // Llena el TreeView con las canciones de la página actual
    for (int i = start; i < end && i < library.size(); ++i) {
        GtkTreeIter iter;
        gtk_list_store_append(listStore, &iter);
        gtk_list_store_set(listStore, &iter, 0, library[i].title.c_str(), 1, library[i].artist.c_str(), 2, library[i].album.c_str(), -1);
    }

    // Actualiza el TreeView
    gtk_tree_view_set_model(GTK_TREE_VIEW(TreeView), GTK_TREE_MODEL(listStore));
}

void paginacion() {
    std::cout << "Paginando\n";
    static int currentPage = 0;

    // Incrementa la página actual y reinicia si se alcanza el final
    currentPage = (currentPage + 1) % ((library.size() + 2) / 3);

    // Pagina y muestra las canciones en el TreeView
    paginateLibrary(currentPage);
}
// Function to send a command to the Arduino
void sendToArduino(LibSerial::SerialStream& arduino, const std::string& message) {
    arduino << message << std::endl;
}

void listenForArduino(LibSerial::SerialStream& arduino) {
    while (true) {
        std::string response;
        std::getline(arduino, response);

        if (response == "P") {
            stop(); // Call the stop() function
        }
    }
}
int main(int argc, char *argv[]) {
    GtkBuilder *builder; // GTK Builder variable
    gtk_init(&argc, &argv); // Start GTK
    LibSerial::SerialStream arduino("/dev/ttyUSB0", LibSerial::SerialStreamBuf::BAUD_9600);
    if (!arduino.IsOpen()) {
        std::cerr << "Error: Couldn't open the Arduino serial port." << std::endl;
        return 1;
    }

    // Thread to listen for Arduino messages in the background
    std::thread listener(listenForArduino, std::ref(arduino));

    arduino.Close();

    //Crear un GtkListStore para almacenar los datos de las canciones
    listStore = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);


    //Cargar la librería al ViewTree
    library = loadLibraryFromCSV(csvFilePath,base,end);
    loadMoreSongs(listStore, library, 0, library.size()); // Cargar todas las canciones

    // Create GTK UI Builder
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "/home/aleprominecraft/Documents/github/OdisseyRadio/Odissey.glade", NULL); // Load our UI file

    //Declaración de widgets
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    memoryLabel = GTK_LABEL(gtk_builder_get_object(builder, "memoryPercent"));
    progressMemory = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "progressMemory"));
    Exit = GTK_WIDGET(gtk_builder_get_object(builder, "Exit"));
    PruebaArduino = GTK_WIDGET(gtk_builder_get_object(builder, "PruebaArduino"));
    Up = GTK_WIDGET(gtk_builder_get_object(builder, "Up"));
    Down = GTK_WIDGET(gtk_builder_get_object(builder, "Down"));
    Pag = GTK_WIDGET(gtk_builder_get_object(builder, "paginar"));

    //TreeView
    TreeView = GTK_WIDGET(gtk_builder_get_object(builder, "TreeView"));
    //TreeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(listStore));
    gtk_tree_view_set_model(GTK_TREE_VIEW(TreeView), GTK_TREE_MODEL(listStore));
    GtkTreeViewColumn* titleColumn = gtk_tree_view_column_new_with_attributes("Título", gtk_cell_renderer_text_new(), "text", 0, NULL);
    GtkTreeViewColumn* artistColumn = gtk_tree_view_column_new_with_attributes("Artista", gtk_cell_renderer_text_new(), "text", 1, NULL);
    GtkTreeViewColumn* albumColumn = gtk_tree_view_column_new_with_attributes("Álbum", gtk_cell_renderer_text_new(), "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), titleColumn);
    gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), artistColumn);
    gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), albumColumn);
    gtk_tree_view_column_set_fixed_width(albumColumn, 140);

    //Declaración de llamadas de funciones
    g_signal_connect(Exit, "clicked", G_CALLBACK(exit_app), NULL);
    g_signal_connect(PruebaArduino, "clicked", G_CALLBACK(pruebaArduino), NULL);
    g_signal_connect(Up, "clicked", G_CALLBACK(subirLista), NULL);
    g_signal_connect(Down, "clicked", G_CALLBACK(bajarLista), NULL);
    g_signal_connect(TreeView, "row-activated", G_CALLBACK(on_row_activated), NULL);
    g_signal_connect(Pag, "row-activated", G_CALLBACK(paginacion), NULL);

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(builder);

    g_timeout_add_seconds(1, reinterpret_cast<GSourceFunc>(updateLabel), memoryLabel);

    gtk_widget_show_all(window); // Show our window
    gtk_main(); // Run our program

    return 0;
}