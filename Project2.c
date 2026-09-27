#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_STUDENTS 5

struct Student
{
    int id;
    char name[50];
    char branch[20];
    int year;
    float feeDue;
    char hostel[50];
};

struct Student students[TOTAL_STUDENTS] =
{
    {101, "Kiran Sri ",  "CSE",  2,     0, "I Block"},
    {102, "Teja", "CSE",  2, 10000, "I Block"},
    {103, "Radha",       "ECE",  3,  3000, "D Block"},
    {104, "Akshay",      "CIVIL",  4,     13000, "J Block"},
    {105, "Akhil",        "EEE", 3,  12500, "J Block"}
};

GtkWidget *entry_id;
GtkWidget *label_result;

void search_student(GtkWidget *widget, gpointer data)
{
    const char *id_text;
    char *end_pointer;
    long entered_id;
    int i;
    int found = 0;
    char result[600];

    (void)widget;
    (void)data;

    id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));

    if (strlen(id_text) == 0)
    {
        gtk_label_set_text(
            GTK_LABEL(label_result),
            "Please enter a Student ID."
        );
        return;
    }

    entered_id = strtol(id_text, &end_pointer, 10);

    if (*end_pointer != '\0' || entered_id <= 0)
    {
        gtk_label_set_text(
            GTK_LABEL(label_result),
            "Invalid ID. Enter numbers only."
        );
        return;
    }

    for (i = 0; i < TOTAL_STUDENTS; i++)
    {
        if (students[i].id == entered_id)
        {
            if (students[i].feeDue == 0)
            {
                snprintf(
                    result,
                    sizeof(result),
                    "STUDENT FOUND\n\n"
                    "ID          : %d\n"
                    "Name        : %s\n"
                    "Branch      : %s\n"
                    "Year        : %d\n"
                    "Hostel      : %s\n"
                    "Fee Status  : PAID",
                    students[i].id,
                    students[i].name,
                    students[i].branch,
                    students[i].year,
                    students[i].hostel
                );
            }
            else
            {
                snprintf(
                    result,
                    sizeof(result),
                    "STUDENT FOUND\n\n"
                    "ID          : %d\n"
                    "Name        : %s\n"
                    "Branch      : %s\n"
                    "Year        : %d\n"
                    "Hostel      : %s\n"
                    "Fee Due     : Rs. %.2f",
                    students[i].id,
                    students[i].name,
                    students[i].branch,
                    students[i].year,
                    students[i].hostel,
                    students[i].feeDue
                );
            }

            gtk_label_set_text(GTK_LABEL(label_result), result);
            found = 1;
            break;
        }
    }

    if (found == 0)
    {
        gtk_label_set_text(
            GTK_LABEL(label_result),
            "STUDENT NOT FOUND\n\n"
            "No student exists with the entered ID."
        );
    }
}

void clear_data(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    gtk_entry_set_text(GTK_ENTRY(entry_id), "");

    gtk_label_set_text(
        GTK_LABEL(label_result),
        "Student details will appear here."
    );

    gtk_widget_grab_focus(entry_id);
}

void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *title_label;
    GtkWidget *instruction_label;
    GtkWidget *button_box;
    GtkWidget *search_button;
    GtkWidget *clear_button;
    GtkWidget *result_frame;

    (void)user_data;

    window = gtk_application_window_new(app);

    gtk_window_set_title(
        GTK_WINDOW(window),
        "RGUKT Student Finder"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        600,
        500
    );

    gtk_container_set_border_width(
        GTK_CONTAINER(window),
        25
    );

    main_box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        15
    );

    gtk_container_add(
        GTK_CONTAINER(window),
        main_box
    );

    title_label = gtk_label_new(
        "RGUKT STUDENT FINDER"
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        title_label,
        FALSE,
        FALSE,
        5
    );

    instruction_label = gtk_label_new(
        "Enter Student ID"
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        instruction_label,
        FALSE,
        FALSE,
        0
    );

    entry_id = gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(entry_id),
        "Example: 103"
    );

    gtk_entry_set_max_length(
        GTK_ENTRY(entry_id),
        10
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        entry_id,
        FALSE,
        FALSE,
        0
    );

    button_box = gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL,
        10
    );

    gtk_box_set_homogeneous(
        GTK_BOX(button_box),
        TRUE
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        button_box,
        FALSE,
        FALSE,
        0
    );

    search_button = gtk_button_new_with_label(
        "SEARCH STUDENT"
    );

    clear_button = gtk_button_new_with_label(
        "CLEAR"
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        search_button,
        TRUE,
        TRUE,
        0
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        clear_button,
        TRUE,
        TRUE,
        0
    );

    result_frame = gtk_frame_new(
        "Search Result"
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        result_frame,
        TRUE,
        TRUE,
        10
    );

    label_result = gtk_label_new(
        "Student details will appear here."
    );

    gtk_label_set_xalign(
        GTK_LABEL(label_result),
        0.0
    );

    gtk_label_set_yalign(
        GTK_LABEL(label_result),
        0.0
    );

    gtk_label_set_selectable(
        GTK_LABEL(label_result),
        TRUE
    );

    gtk_container_set_border_width(
        GTK_CONTAINER(label_result),
        20
    );

    gtk_container_add(
        GTK_CONTAINER(result_frame),
        label_result
    );

    g_signal_connect(
        search_button,
        "clicked",
        G_CALLBACK(search_student),
        NULL
    );

    g_signal_connect(
        clear_button,
        "clicked",
        G_CALLBACK(clear_data),
        NULL
    );

    g_signal_connect(
        entry_id,
        "activate",
        G_CALLBACK(search_student),
        NULL
    );

    gtk_widget_show_all(window);

    gtk_widget_grab_focus(entry_id);
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new(
        "in.ac.rgukt.studentfinder",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(
        app,
        "activate",
        G_CALLBACK(activate),
        NULL
    );

    status = g_application_run(
        G_APPLICATION(app),
        argc,
        argv
    );

    g_object_unref(app);

    return status;
}
