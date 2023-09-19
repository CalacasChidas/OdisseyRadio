#include <gtk/gtk.h>
#include <stdio.h>
#include <iostream>
GtkWidget *window, *play;
void play_(GtkButton *button, gpointer user_data){
    std::cout << "Playing :D\n";
}
int main(int argc, char *argv[]) {
    GtkBuilder *builder; // GTK Builder variable
    gtk_init(&argc, &argv); // Start GTK

    builder = gtk_builder_new(); // Create GTK UI Builder
    gtk_builder_add_from_file(builder, "/home/aleprominecraft/Documents/github/OdisseyRadio/Odissey.glade", NULL); // Load our UI file

    //Declaración de widgets
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    play = GTK_WIDGET(gtk_builder_get_object(builder, "play"));

    //Declaración de llamadas de funciones
    g_signal_connect(play, "clicked", G_CALLBACK(play_), NULL);

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(builder);

    gtk_widget_show_all(window); // Show our window
    gtk_main(); // Run our program

    return 0;
}
