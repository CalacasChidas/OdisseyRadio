#include <gtk/gtk.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

struct Song {
    std::string title;
    std::string artist;
    std::string album;
};

GtkWidget *window, *PruebaArduino, *Exit, *TreeView;
GtkLabel *memoryLabel;
GtkProgressBar *progressMemory;

std::vector<Song> loadLibraryFromCSV(const std::string& csvFilePath) {
    std::vector<Song> library;

    // Abre el archivo CSV
    std::ifstream csvFile(csvFilePath);
    std::cout << "Done!\n";
    if (!csvFile.is_open()) {
        std::cerr << "Error al abrir el archivo CSV" << std::endl;
        return library;
    }

    std::string line;
    while (std::getline(csvFile, line)) {
        //std::cout<<"Im doing somehting\n";
        std::istringstream iss(line);
        std::string title, artist, album;
        if (std::getline(iss, title, ',') &&
            std::getline(iss, artist, ',') &&
            std::getline(iss, album)) {
            library.push_back({title, artist, album});
        }
    }

    // Cierra el archivo CSV
    csvFile.close();

    return library;
}
void pruebaArduino(){
    std::cout << "Si funciona!\n";
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


int main(int argc, char *argv[]) {
    GtkBuilder *builder; // GTK Builder variable
    gtk_init(&argc, &argv); // Start GTK

    //Crear un GtkListStore para almacenar los datos de las canciones
    GtkListStore *listStore = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    std::string csvFilePath = "/home/aleprominecraft/fma/data/fma_metadata/tracks.csv";
    std::vector<Song> library = loadLibraryFromCSV(csvFilePath);
    /*
    for (const Song& song : library) {
        std::cout << "Título: " << song.title << ", Artista: " << song.artist << ", Álbum: " << song.album << std::endl;
    }
     */
    for (const Song& song : library) {
        GtkTreeIter iter;
        gtk_list_store_append(listStore, &iter);
        gtk_list_store_set(listStore, &iter, 0, song.title.c_str(), 1, song.artist.c_str(), 2, song.album.c_str(), -1);
    }


    builder = gtk_builder_new(); // Create GTK UI Builder
    gtk_builder_add_from_file(builder, "/home/aleprominecraft/Documents/github/OdisseyRadio/Odissey.glade", NULL); // Load our UI file

    //Declaración de widgets
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    memoryLabel = GTK_LABEL(gtk_builder_get_object(builder, "memoryPercent"));
    progressMemory = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "progressMemory"));
    Exit = GTK_WIDGET(gtk_builder_get_object(builder, "Exit"));
    PruebaArduino = GTK_WIDGET(gtk_builder_get_object(builder, "PruebaArduino"));
    TreeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(listStore));

    for (int i = 0; i < 3; ++i) {
        GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
                (i == 0) ? "Título" : (i == 1) ? "Artista" : "Álbum",
                renderer,
                "text", i,
                NULL
        );
        gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), column);
    }

    //Declaración de llamadas de funciones
    g_signal_connect(Exit, "clicked", G_CALLBACK(exit_app), NULL);
    g_signal_connect(PruebaArduino, "clicked", G_CALLBACK(pruebaArduino), NULL);

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(builder);

    g_timeout_add_seconds(1, reinterpret_cast<GSourceFunc>(updateLabel), memoryLabel);


    gtk_widget_show_all(window); // Show our window
    gtk_main(); // Run our program


    return 0;
}
