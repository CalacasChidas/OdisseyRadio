#include <gtk/gtk.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

GtkWidget *window, *play;
GtkLabel *memoryLabel;
GtkProgressBar *progressMemory;


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


int main(int argc, char *argv[]) {
    GtkBuilder *builder; // GTK Builder variable
    gtk_init(&argc, &argv); // Start GTK

    builder = gtk_builder_new(); // Create GTK UI Builder
    gtk_builder_add_from_file(builder, "/home/aleprominecraft/Documents/github/OdisseyRadio/Odissey.glade", NULL); // Load our UI file

    //Declaración de widgets
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    memoryLabel = GTK_LABEL(gtk_builder_get_object(builder, "memoryPercent"));
    progressMemory = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "progressMemory"));

    //play = GTK_WIDGET(gtk_builder_get_object(builder, "play"));

    //Declaración de llamadas de funciones
    //g_signal_connect(play, "clicked", G_CALLBACK(play_), NULL);

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(builder);

    g_timeout_add_seconds(1, reinterpret_cast<GSourceFunc>(updateLabel), memoryLabel);

    gtk_widget_show_all(window); // Show our window
    gtk_main(); // Run our program


    return 0;
}
