#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DB_FILE "student_database.db"

/* ---------- Global Variables ---------- */

sqlite3 *db = NULL;

GtkWidget *login_window = NULL;
GtkWidget *entry_username = NULL;
GtkWidget *entry_password = NULL;

GtkWidget *dashboard_window = NULL;

GtkWidget *entry_student_id = NULL;
GtkWidget *entry_name = NULL;
GtkWidget *entry_branch = NULL;
GtkWidget *entry_year = NULL;
GtkWidget *combo_scholarship = NULL;
GtkWidget *entry_total_fee = NULL;
GtkWidget *entry_paid_fee = NULL;

GtkWidget *tree_view = NULL;
GtkListStore *student_store = NULL;

/*
 * Used when switching between login and dashboard.
 *
 * Without this flag:
 *
 * Login -> destroy login window -> gtk_main_quit()
 *
 * which terminates the entire application.
 */
gboolean switching_windows = FALSE;


/* ---------- Function Declarations ---------- */

void show_message(GtkWindow *parent,
                  const char *title,
                  const char *message,
                  GtkMessageType type);

void apply_css();

void open_database();
void create_tables();
void create_default_admin();

int check_login(const char *username, const char *password);
int student_exists(const char *student_id);

void show_login_window();
void show_dashboard_window();

void clear_student_form();
void refresh_student_table(const char *specific_id);

void set_combo_scholarship(const char *value);
const char *get_combo_scholarship();

void on_login_clicked(GtkWidget *widget, gpointer data);

void on_add_clicked(GtkWidget *widget, gpointer data);
void on_update_clicked(GtkWidget *widget, gpointer data);
void on_delete_clicked(GtkWidget *widget, gpointer data);
void on_search_clicked(GtkWidget *widget, gpointer data);
void on_clear_clicked(GtkWidget *widget, gpointer data);
void on_refresh_clicked(GtkWidget *widget, gpointer data);
void on_logout_clicked(GtkWidget *widget, gpointer data);

void on_tree_selection_changed(GtkTreeSelection *selection,
                               gpointer data);

void on_login_window_destroy(GtkWidget *widget, gpointer data);
void on_dashboard_window_destroy(GtkWidget *widget, gpointer data);


/* =========================================================
   Utility Functions
   ========================================================= */

void apply_css()
{
    GtkCssProvider *provider;

    provider = gtk_css_provider_new();

    gtk_css_provider_load_from_data(
        provider,

        "window {"
        "    background: #F4F6F7;"
        "}"

        "#title_label {"
        "    font-size: 24px;"
        "    font-weight: bold;"
        "    color: #1A5276;"
        "}"

        "#small_title_label {"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    color: #1A5276;"
        "}"

        "button {"
        "    padding: 8px;"
        "    font-weight: bold;"
        "}"

        "entry {"
        "    padding: 5px;"
        "}"

        "treeview {"
        "    font-size: 12px;"
        "}",

        -1,
        NULL
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}


/* ---------- Message Dialog ---------- */

void show_message(GtkWindow *parent,
                  const char *title,
                  const char *message,
                  GtkMessageType type)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        type,
        GTK_BUTTONS_OK,
        "%s",
        message
    );

    gtk_window_set_title(GTK_WINDOW(dialog), title);

    gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
}


/* ---------- Empty String Check ---------- */

int is_empty(const char *text)
{
    return text == NULL || strlen(text) == 0;
}


/* =========================================================
   Scholarship Functions
   ========================================================= */

const char *get_combo_scholarship()
{
    int active_index;

    active_index =
        gtk_combo_box_get_active(
            GTK_COMBO_BOX(combo_scholarship)
        );

    if (active_index == 1)
    {
        return "Non-Scholarship";
    }

    return "Scholarship";
}


void set_combo_scholarship(const char *value)
{
    if (strcmp(value, "Scholarship") == 0)
    {
        gtk_combo_box_set_active(
            GTK_COMBO_BOX(combo_scholarship),
            0
        );
    }
    else
    {
        gtk_combo_box_set_active(
            GTK_COMBO_BOX(combo_scholarship),
            1
        );
    }
}


/* =========================================================
   Database Functions
   ========================================================= */

void open_database()
{
    int result;

    result = sqlite3_open(DB_FILE, &db);

    if (result != SQLITE_OK)
    {
        printf(
            "Database opening failed: %s\n",
            sqlite3_errmsg(db)
        );

        sqlite3_close(db);
        db = NULL;

        exit(1);
    }

    printf("Database opened successfully.\n");
}


/* ---------- Create Tables ---------- */

void create_tables()
{
    char *error_message = NULL;

    const char *users_table =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "role TEXT NOT NULL"
        ");";

    const char *students_table =
        "CREATE TABLE IF NOT EXISTS students ("
        "student_id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "branch TEXT NOT NULL,"
        "year TEXT NOT NULL,"
        "scholarship TEXT NOT NULL,"
        "total_fee REAL NOT NULL,"
        "paid_fee REAL NOT NULL"
        ");";

    if (sqlite3_exec(
            db,
            users_table,
            NULL,
            NULL,
            &error_message
        ) != SQLITE_OK)
    {
        printf(
            "Error creating users table: %s\n",
            error_message
        );

        sqlite3_free(error_message);
        error_message = NULL;
    }

    if (sqlite3_exec(
            db,
            students_table,
            NULL,
            NULL,
            &error_message
        ) != SQLITE_OK)
    {
        printf(
            "Error creating students table: %s\n",
            error_message
        );

        sqlite3_free(error_message);
        error_message = NULL;
    }
}


/* ---------- Create Default Admin ---------- */

void create_default_admin()
{
    char *error_message = NULL;

    const char *insert_admin =
        "INSERT OR IGNORE INTO users"
        "(username, password, role) "
        "VALUES"
        "('admin', 'admin123', 'Admin');";

    if (sqlite3_exec(
            db,
            insert_admin,
            NULL,
            NULL,
            &error_message
        ) != SQLITE_OK)
    {
        printf(
            "Error creating default admin: %s\n",
            error_message
        );

        sqlite3_free(error_message);
    }
}


/* ---------- Check Login ---------- */

int check_login(const char *username,
                const char *password)
{
    sqlite3_stmt *stmt = NULL;

    int login_success = 0;

    const char *sql =
        "SELECT COUNT(*) "
        "FROM users "
        "WHERE username = ? AND password = ?;";

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            NULL
        ) != SQLITE_OK)
    {
        printf(
            "Login query preparation failed: %s\n",
            sqlite3_errmsg(db)
        );

        return 0;
    }

    sqlite3_bind_text(
        stmt,
        1,
        username,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        2,
        password,
        -1,
        SQLITE_TRANSIENT
    );

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (sqlite3_column_int(stmt, 0) > 0)
        {
            login_success = 1;
        }
    }

    sqlite3_finalize(stmt);

    return login_success;
}


/* ---------- Check Student Exists ---------- */

int student_exists(const char *student_id)
{
    sqlite3_stmt *stmt = NULL;

    int exists = 0;

    const char *sql =
        "SELECT COUNT(*) "
        "FROM students "
        "WHERE student_id = ?;";

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            NULL
        ) != SQLITE_OK)
    {
        return 0;
    }

    sqlite3_bind_text(
        stmt,
        1,
        student_id,
        -1,
        SQLITE_TRANSIENT
    );

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (sqlite3_column_int(stmt, 0) > 0)
        {
            exists = 1;
        }
    }

    sqlite3_finalize(stmt);

    return exists;
}


/* =========================================================
   LOGIN WINDOW
   ========================================================= */

/*
 * This function is called when the login window is destroyed.
 *
 * If the user manually closes the login window,
 * the application should terminate.
 *
 * But when we are moving from Login -> Dashboard,
 * we DO NOT terminate the application.
 */
void on_login_window_destroy(GtkWidget *widget,
                             gpointer data)
{
    if (!switching_windows)
    {
        gtk_main_quit();
    }
}


/* ---------- Show Login Window ---------- */

void show_login_window()
{
    GtkWidget *main_box;
    GtkWidget *title_label;
    GtkWidget *grid;

    GtkWidget *label_username;
    GtkWidget *label_password;

    GtkWidget *login_button;
    GtkWidget *note_label;

    login_window =
        gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(
        GTK_WINDOW(login_window),
        "Student Database Login"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(login_window),
        420,
        260
    );

    gtk_window_set_position(
        GTK_WINDOW(login_window),
        GTK_WIN_POS_CENTER
    );

    gtk_container_set_border_width(
        GTK_CONTAINER(login_window),
        20
    );

    /*
     * IMPORTANT:
     *
     * Do NOT directly connect this to gtk_main_quit().
     * Otherwise successful login would terminate the program.
     */
    g_signal_connect(
        login_window,
        "destroy",
        G_CALLBACK(on_login_window_destroy),
        NULL
    );


    /* Main vertical box */

    main_box =
        gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            12
        );

    gtk_container_add(
        GTK_CONTAINER(login_window),
        main_box
    );


    /* Title */

    title_label =
        gtk_label_new(
            "STUDENT DATABASE LOGIN"
        );

    gtk_widget_set_name(
        title_label,
        "title_label"
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        title_label,
        FALSE,
        FALSE,
        5
    );


    /* Grid */

    grid = gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(grid),
        10
    );

    gtk_grid_set_column_spacing(
        GTK_GRID(grid),
        10
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        grid,
        FALSE,
        FALSE,
        5
    );


    /* Labels */

    label_username =
        gtk_label_new("Username:");

    label_password =
        gtk_label_new("Password:");


    /* Entries */

    entry_username =
        gtk_entry_new();

    entry_password =
        gtk_entry_new();


    /* Password hidden */

    gtk_entry_set_visibility(
        GTK_ENTRY(entry_password),
        FALSE
    );

    gtk_entry_set_invisible_char(
        GTK_ENTRY(entry_password),
        '*'
    );


    /* Attach */

    gtk_grid_attach(
        GTK_GRID(grid),
        label_username,
        0,
        0,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(grid),
        entry_username,
        1,
        0,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(grid),
        label_password,
        0,
        1,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(grid),
        entry_password,
        1,
        1,
        1,
        1
    );


    /* Login button */

    login_button =
        gtk_button_new_with_label("Login");

    gtk_box_pack_start(
        GTK_BOX(main_box),
        login_button,
        FALSE,
        FALSE,
        5
    );

    g_signal_connect(
        login_button,
        "clicked",
        G_CALLBACK(on_login_clicked),
        NULL
    );


    /* Default credentials */

    note_label =
        gtk_label_new(
            "Default login: username = admin, password = admin123"
        );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        note_label,
        FALSE,
        FALSE,
        5
    );


    gtk_widget_show_all(login_window);
}


/* ---------- Login Button ---------- */

void on_login_clicked(GtkWidget *widget,
                      gpointer data)
{
    const char *username;
    const char *password;


    username =
        gtk_entry_get_text(
            GTK_ENTRY(entry_username)
        );

    password =
        gtk_entry_get_text(
            GTK_ENTRY(entry_password)
        );


    /* Empty validation */

    if (is_empty(username) ||
        is_empty(password))
    {
        show_message(
            GTK_WINDOW(login_window),
            "Login Error",
            "Please enter username and password.",
            GTK_MESSAGE_WARNING
        );

        return;
    }


    /* Check credentials */

    if (check_login(username, password))
    {
        /*
         * Tell destroy callbacks that we are
         * switching windows and NOT exiting.
         */
        switching_windows = TRUE;


        /*
         * Create dashboard first.
         */
        show_dashboard_window();


        /*
         * Now destroy login window.
         */
        if (login_window != NULL)
        {
            gtk_widget_destroy(login_window);
            login_window = NULL;
        }


        /*
         * Re-enable normal destroy behavior.
         */
        switching_windows = FALSE;
    }
    else
    {
        show_message(
            GTK_WINDOW(login_window),
            "Login Failed",
            "Invalid username or password.",
            GTK_MESSAGE_ERROR
        );
    }
}


/* =========================================================
   DASHBOARD WINDOW
   ========================================================= */

/*
 * Dashboard destroy callback.
 *
 * If the user clicks the X button,
 * the application should exit.
 *
 * If logout is happening, we don't exit.
 */
void on_dashboard_window_destroy(GtkWidget *widget,
                                 gpointer data)
{
    if (!switching_windows)
    {
        gtk_main_quit();
    }
}


/* ---------- Show Dashboard ---------- */

void show_dashboard_window()
{
    GtkWidget *main_box;
    GtkWidget *title_label;

    GtkWidget *form_frame;
    GtkWidget *form_grid;

    GtkWidget *button_box;

    GtkWidget *add_button;
    GtkWidget *update_button;
    GtkWidget *delete_button;
    GtkWidget *search_button;
    GtkWidget *clear_button;
    GtkWidget *refresh_button;
    GtkWidget *logout_button;

    GtkWidget *scroll_window;

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;


    dashboard_window =
        gtk_window_new(GTK_WINDOW_TOPLEVEL);


    gtk_window_set_title(
        GTK_WINDOW(dashboard_window),
        "Student Database Management System"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(dashboard_window),
        1050,
        650
    );

    gtk_window_set_position(
        GTK_WINDOW(dashboard_window),
        GTK_WIN_POS_CENTER
    );

    gtk_container_set_border_width(
        GTK_CONTAINER(dashboard_window),
        12
    );


    g_signal_connect(
        dashboard_window,
        "destroy",
        G_CALLBACK(on_dashboard_window_destroy),
        NULL
    );


    /* Main box */

    main_box =
        gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            10
        );

    gtk_container_add(
        GTK_CONTAINER(dashboard_window),
        main_box
    );


    /* Title */

    title_label =
        gtk_label_new(
            "STUDENT DATABASE MANAGEMENT SYSTEM"
        );

    gtk_widget_set_name(
        title_label,
        "small_title_label"
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        title_label,
        FALSE,
        FALSE,
        5
    );


    /* Student information frame */

    form_frame =
        gtk_frame_new(
            "Student Information"
        );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        form_frame,
        FALSE,
        FALSE,
        5
    );


    /* Form grid */

    form_grid =
        gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(form_grid),
        8
    );

    gtk_grid_set_column_spacing(
        GTK_GRID(form_grid),
        10
    );

    gtk_container_set_border_width(
        GTK_CONTAINER(form_grid),
        10
    );

    gtk_container_add(
        GTK_CONTAINER(form_frame),
        form_grid
    );


    /* Entries */

    entry_student_id =
        gtk_entry_new();

    entry_name =
        gtk_entry_new();

    entry_branch =
        gtk_entry_new();

    entry_year =
        gtk_entry_new();


    combo_scholarship =
        gtk_combo_box_text_new();


    entry_total_fee =
        gtk_entry_new();

    entry_paid_fee =
        gtk_entry_new();


    /* Scholarship */

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(combo_scholarship),
        "Scholarship"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(combo_scholarship),
        "Non-Scholarship"
    );

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(combo_scholarship),
        0
    );


    /* Row 0 */

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Student ID:"),
        0,
        0,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_student_id,
        1,
        0,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Name:"),
        2,
        0,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_name,
        3,
        0,
        1,
        1
    );


    /* Row 1 */

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Branch:"),
        0,
        1,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_branch,
        1,
        1,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Year:"),
        2,
        1,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_year,
        3,
        1,
        1,
        1
    );


    /* Row 2 */

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Scholarship:"),
        0,
        2,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        combo_scholarship,
        1,
        2,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Total Fee:"),
        2,
        2,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_total_fee,
        3,
        2,
        1,
        1
    );


    /* Row 3 */

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Paid Fee:"),
        0,
        3,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_paid_fee,
        1,
        3,
        1,
        1
    );


    /* =====================================================
       Buttons
       ===================================================== */

    button_box =
        gtk_box_new(
            GTK_ORIENTATION_HORIZONTAL,
            8
        );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        button_box,
        FALSE,
        FALSE,
        5
    );


    add_button =
        gtk_button_new_with_label(
            "Add Student"
        );

    update_button =
        gtk_button_new_with_label(
            "Update"
        );

    delete_button =
        gtk_button_new_with_label(
            "Delete"
        );

    search_button =
        gtk_button_new_with_label(
            "Search by ID"
        );

    clear_button =
        gtk_button_new_with_label(
            "Clear Form"
        );

    refresh_button =
        gtk_button_new_with_label(
            "Refresh Table"
        );

    logout_button =
        gtk_button_new_with_label(
            "Logout"
        );


    gtk_box_pack_start(
        GTK_BOX(button_box),
        add_button,
        TRUE,
        TRUE,
        2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        update_button,
        TRUE,
        TRUE,
        2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        delete_button,
        TRUE,
        TRUE,
        2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        search_button,
        TRUE,
        TRUE,
        2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        clear_button,
        TRUE,
        TRUE,
        2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        refresh_button,
        TRUE,
        TRUE,
        2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        logout_button,
        TRUE,
        TRUE,
        2
    );


    /* Signals */

    g_signal_connect(
        add_button,
        "clicked",
        G_CALLBACK(on_add_clicked),
        NULL
    );

    g_signal_connect(
        update_button,
        "clicked",
        G_CALLBACK(on_update_clicked),
        NULL
    );

    g_signal_connect(
        delete_button,
        "clicked",
        G_CALLBACK(on_delete_clicked),
        NULL
    );

    g_signal_connect(
        search_button,
        "clicked",
        G_CALLBACK(on_search_clicked),
        NULL
    );

    g_signal_connect(
        clear_button,
        "clicked",
        G_CALLBACK(on_clear_clicked),
        NULL
    );

    g_signal_connect(
        refresh_button,
        "clicked",
        G_CALLBACK(on_refresh_clicked),
        NULL
    );

    g_signal_connect(
        logout_button,
        "clicked",
        G_CALLBACK(on_logout_clicked),
        NULL
    );


    /* =====================================================
       Student Table
       ===================================================== */

    student_store =
        gtk_list_store_new(
            8,

            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_STRING
        );


    tree_view =
        gtk_tree_view_new_with_model(
            GTK_TREE_MODEL(student_store)
        );


    gtk_tree_view_set_grid_lines(
        GTK_TREE_VIEW(tree_view),
        GTK_TREE_VIEW_GRID_LINES_BOTH
    );


    /* Student ID */

    renderer =
        gtk_cell_renderer_text_new();

    column =
        gtk_tree_view_column_new_with_attributes(
            "Student ID",
            renderer,
            "text",
            0,
            NULL
        );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(tree_view),
        column
    );


    /* Name */

    renderer =
        gtk_cell_renderer_text_new();

    column =
        gtk_tree_view_column_new_with_attributes(
            "Name",
            renderer,
            "text",
            1,
            NULL
        );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(tree_view),
        column
    );


    /* Branch */

    renderer =
        gtk_cell_renderer_text_new();

    column =
        gtk_tree_view_column_new_with_attributes(
            "Branch",
            renderer,
            "text",
            2,
            NULL
        );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(tree_view),
        column
    );


    /* Year */

    renderer =
        gtk_cell_renderer_text_new();

    column =
        gtk_tree_view_column_new_with_attributes(
            "Year",
            renderer,
            "text",
            3,
            NULL
        );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(tree_view),
        column
    );


    /* Scholarship */

    renderer =
        gtk_cell_renderer_text_new();

    column =
        gtk_tree_view_column_new_with_attributes(
            "Scholarship",
            renderer,
            "text",
            4,
            NULL
        );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(tree_view),
        column
    );


    /* Total Fee */

    renderer =
        gtk_cell_renderer_text_new();

    column =
        gtk_tree_view_column_new_with_attributes(
            "Total Fee",
            renderer,
            "text",
            5,
            NULL
        );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(tree_view),
        column
    );


    /* Paid Fee */

    renderer =
        gtk_cell_renderer_text_new();

    column =
        gtk_tree_view_column_new_with_attributes(
            "Paid Fee",
            renderer,
            "text",
            6,
            NULL
        );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(tree_view),
        column
    );


    /* Pending Fee */

    renderer =
        gtk_cell_renderer_text_new();

    column =
        gtk_tree_view_column_new_with_attributes(
            "Pending Fee",
            renderer,
            "text",
            7,
            NULL
        );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(tree_view),
        column
    );


    /* Selection */

    selection =
        gtk_tree_view_get_selection(
            GTK_TREE_VIEW(tree_view)
        );

    g_signal_connect(
        selection,
        "changed",
        G_CALLBACK(on_tree_selection_changed),
        NULL
    );


    /* Scroll window */

    scroll_window =
        gtk_scrolled_window_new(
            NULL,
            NULL
        );

    gtk_widget_set_vexpand(
        scroll_window,
        TRUE
    );

    gtk_container_add(
        GTK_CONTAINER(scroll_window),
        tree_view
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        scroll_window,
        TRUE,
        TRUE,
        5
    );


    /* Load existing students */

    refresh_student_table(NULL);


    /* Show dashboard */

    gtk_widget_show_all(
        dashboard_window
    );
}


/* =========================================================
   Student Operations
   ========================================================= */

void clear_student_form()
{
    gtk_entry_set_text(
        GTK_ENTRY(entry_student_id),
        ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(entry_name),
        ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(entry_branch),
        ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(entry_year),
        ""
    );

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(combo_scholarship),
        0
    );

    gtk_entry_set_text(
        GTK_ENTRY(entry_total_fee),
        ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(entry_paid_fee),
        ""
    );
}


/* =========================================================
   ADD STUDENT
   ========================================================= */

void on_add_clicked(GtkWidget *widget,
                    gpointer data)
{
    sqlite3_stmt *stmt = NULL;

    const char *student_id;
    const char *name;
    const char *branch;
    const char *year;
    const char *scholarship;

    double total_fee;
    double paid_fee;

    const char *sql =
        "INSERT INTO students"
        "(student_id, name, branch, year, scholarship, total_fee, paid_fee) "
        "VALUES(?, ?, ?, ?, ?, ?, ?);";


    student_id =
        gtk_entry_get_text(
            GTK_ENTRY(entry_student_id)
        );

    name =
        gtk_entry_get_text(
            GTK_ENTRY(entry_name)
        );

    branch =
        gtk_entry_get_text(
            GTK_ENTRY(entry_branch)
        );

    year =
        gtk_entry_get_text(
            GTK_ENTRY(entry_year)
        );

    scholarship =
        get_combo_scholarship();


    total_fee =
        atof(
            gtk_entry_get_text(
                GTK_ENTRY(entry_total_fee)
            )
        );

    paid_fee =
        atof(
            gtk_entry_get_text(
                GTK_ENTRY(entry_paid_fee)
            )
        );


    if (is_empty(student_id) ||
        is_empty(name) ||
        is_empty(branch) ||
        is_empty(year))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Missing Data",
            "Please fill Student ID, Name, Branch and Year.",
            GTK_MESSAGE_WARNING
        );

        return;
    }


    if (student_exists(student_id))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Duplicate ID",
            "Student ID already exists. Use Update instead.",
            GTK_MESSAGE_ERROR
        );

        return;
    }


    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            NULL
        ) != SQLITE_OK)
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Database Error",
            "Unable to prepare insert query.",
            GTK_MESSAGE_ERROR
        );

        return;
    }


    sqlite3_bind_text(
        stmt,
        1,
        student_id,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        2,
        name,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        3,
        branch,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        4,
        year,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        5,
        scholarship,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_double(
        stmt,
        6,
        total_fee
    );

    sqlite3_bind_double(
        stmt,
        7,
        paid_fee
    );


    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Success",
            "Student record added successfully.",
            GTK_MESSAGE_INFO
        );

        clear_student_form();

        refresh_student_table(NULL);
    }
    else
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Insert Failed",
            sqlite3_errmsg(db),
            GTK_MESSAGE_ERROR
        );
    }


    sqlite3_finalize(stmt);
}


/* =========================================================
   UPDATE STUDENT
   ========================================================= */

void on_update_clicked(GtkWidget *widget,
                       gpointer data)
{
    sqlite3_stmt *stmt = NULL;

    const char *student_id;
    const char *name;
    const char *branch;
    const char *year;
    const char *scholarship;

    double total_fee;
    double paid_fee;

    const char *sql =
        "UPDATE students "
        "SET name = ?, "
        "branch = ?, "
        "year = ?, "
        "scholarship = ?, "
        "total_fee = ?, "
        "paid_fee = ? "
        "WHERE student_id = ?;";


    student_id =
        gtk_entry_get_text(
            GTK_ENTRY(entry_student_id)
        );

    name =
        gtk_entry_get_text(
            GTK_ENTRY(entry_name)
        );

    branch =
        gtk_entry_get_text(
            GTK_ENTRY(entry_branch)
        );

    year =
        gtk_entry_get_text(
            GTK_ENTRY(entry_year)
        );

    scholarship =
        get_combo_scholarship();


    total_fee =
        atof(
            gtk_entry_get_text(
                GTK_ENTRY(entry_total_fee)
            )
        );

    paid_fee =
        atof(
            gtk_entry_get_text(
                GTK_ENTRY(entry_paid_fee)
            )
        );


    if (is_empty(student_id))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Missing ID",
            "Enter Student ID or select a record from the table.",
            GTK_MESSAGE_WARNING
        );

        return;
    }


    if (!student_exists(student_id))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Not Found",
            "Student ID not found. Add the record first.",
            GTK_MESSAGE_ERROR
        );

        return;
    }


    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            NULL
        ) != SQLITE_OK)
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Database Error",
            "Unable to prepare update query.",
            GTK_MESSAGE_ERROR
        );

        return;
    }


    sqlite3_bind_text(
        stmt,
        1,
        name,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        2,
        branch,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        3,
        year,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt,
        4,
        scholarship,
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_double(
        stmt,
        5,
        total_fee
    );

    sqlite3_bind_double(
        stmt,
        6,
        paid_fee
    );

    sqlite3_bind_text(
        stmt,
        7,
        student_id,
        -1,
        SQLITE_TRANSIENT
    );


    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Success",
            "Student record updated successfully.",
            GTK_MESSAGE_INFO
        );

        refresh_student_table(NULL);
    }
    else
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Update Failed",
            sqlite3_errmsg(db),
            GTK_MESSAGE_ERROR
        );
    }


    sqlite3_finalize(stmt);
}


/* =========================================================
   DELETE STUDENT
   ========================================================= */

void on_delete_clicked(GtkWidget *widget,
                       gpointer data)
{
    sqlite3_stmt *stmt = NULL;

    const char *student_id;

    GtkWidget *dialog;

    int response;

    const char *sql =
        "DELETE FROM students "
        "WHERE student_id = ?;";


    student_id =
        gtk_entry_get_text(
            GTK_ENTRY(entry_student_id)
        );


    if (is_empty(student_id))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Missing ID",
            "Enter Student ID or select a record from the table.",
            GTK_MESSAGE_WARNING
        );

        return;
    }


    if (!student_exists(student_id))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Not Found",
            "Student ID not found.",
            GTK_MESSAGE_ERROR
        );

        return;
    }


    dialog =
        gtk_message_dialog_new(
            GTK_WINDOW(dashboard_window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "Are you sure you want to delete Student ID: %s?",
            student_id
        );


    response =
        gtk_dialog_run(
            GTK_DIALOG(dialog)
        );


    gtk_widget_destroy(dialog);


    if (response != GTK_RESPONSE_YES)
    {
        return;
    }


    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            NULL
        ) != SQLITE_OK)
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Database Error",
            "Unable to prepare delete query.",
            GTK_MESSAGE_ERROR
        );

        return;
    }


    sqlite3_bind_text(
        stmt,
        1,
        student_id,
        -1,
        SQLITE_TRANSIENT
    );


    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Deleted",
            "Student record deleted successfully.",
            GTK_MESSAGE_INFO
        );

        clear_student_form();

        refresh_student_table(NULL);
    }
    else
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Delete Failed",
            sqlite3_errmsg(db),
            GTK_MESSAGE_ERROR
        );
    }


    sqlite3_finalize(stmt);
}


/* =========================================================
   SEARCH STUDENT
   ========================================================= */

void on_search_clicked(GtkWidget *widget,
                       gpointer data)
{
    const char *student_id;


    student_id =
        gtk_entry_get_text(
            GTK_ENTRY(entry_student_id)
        );


    if (is_empty(student_id))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Missing ID",
            "Enter Student ID to search.",
            GTK_MESSAGE_WARNING
        );

        return;
    }


    if (!student_exists(student_id))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Not Found",
            "No student found with this ID.",
            GTK_MESSAGE_ERROR
        );

        refresh_student_table(NULL);

        return;
    }


    refresh_student_table(student_id);
}


/* =========================================================
   CLEAR
   ========================================================= */

void on_clear_clicked(GtkWidget *widget,
                      gpointer data)
{
    clear_student_form();
}


/* =========================================================
   REFRESH
   ========================================================= */

void on_refresh_clicked(GtkWidget *widget,
                        gpointer data)
{
    refresh_student_table(NULL);
}


/* =========================================================
   LOGOUT
   ========================================================= */

void on_logout_clicked(GtkWidget *widget,
                       gpointer data)
{
    /*
     * We are switching from Dashboard -> Login.
     *
     * Therefore the dashboard destroy callback must
     * NOT call gtk_main_quit().
     */

    switching_windows = TRUE;


    if (dashboard_window != NULL)
    {
        gtk_widget_destroy(dashboard_window);
        dashboard_window = NULL;
    }


    show_login_window();


    switching_windows = FALSE;
}


/* =========================================================
   REFRESH STUDENT TABLE
   ========================================================= */

void refresh_student_table(const char *specific_id)
{
    sqlite3_stmt *stmt = NULL;

    GtkTreeIter iter;

    char total_text[30];
    char paid_text[30];
    char pending_text[30];


    const char *sql_all =
        "SELECT student_id, "
        "name, "
        "branch, "
        "year, "
        "scholarship, "
        "total_fee, "
        "paid_fee, "
        "(total_fee - paid_fee) AS pending_fee "
        "FROM students "
        "ORDER BY student_id;";


    const char *sql_one =
        "SELECT student_id, "
        "name, "
        "branch, "
        "year, "
        "scholarship, "
        "total_fee, "
        "paid_fee, "
        "(total_fee - paid_fee) AS pending_fee "
        "FROM students "
        "WHERE student_id = ?;";


    if (student_store == NULL)
    {
        return;
    }


    gtk_list_store_clear(student_store);


    /* Get all students */

    if (specific_id == NULL ||
        strlen(specific_id) == 0)
    {
        if (sqlite3_prepare_v2(
                db,
                sql_all,
                -1,
                &stmt,
                NULL
            ) != SQLITE_OK)
        {
            printf(
                "Error preparing student query: %s\n",
                sqlite3_errmsg(db)
            );

            return;
        }
    }

    /* Get one student */

    else
    {
        if (sqlite3_prepare_v2(
                db,
                sql_one,
                -1,
                &stmt,
                NULL
            ) != SQLITE_OK)
        {
            printf(
                "Error preparing student search: %s\n",
                sqlite3_errmsg(db)
            );

            return;
        }


        sqlite3_bind_text(
            stmt,
            1,
            specific_id,
            -1,
            SQLITE_TRANSIENT
        );
    }


    /* Read rows */

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char *student_id;
        const unsigned char *name;
        const unsigned char *branch;
        const unsigned char *year;
        const unsigned char *scholarship;

        double total_fee;
        double paid_fee;
        double pending_fee;


        student_id =
            sqlite3_column_text(stmt, 0);

        name =
            sqlite3_column_text(stmt, 1);

        branch =
            sqlite3_column_text(stmt, 2);

        year =
            sqlite3_column_text(stmt, 3);

        scholarship =
            sqlite3_column_text(stmt, 4);


        total_fee =
            sqlite3_column_double(stmt, 5);

        paid_fee =
            sqlite3_column_double(stmt, 6);

        pending_fee =
            sqlite3_column_double(stmt, 7);


        snprintf(
            total_text,
            sizeof(total_text),
            "%.2f",
            total_fee
        );

        snprintf(
            paid_text,
            sizeof(paid_text),
            "%.2f",
            paid_fee
        );

        snprintf(
            pending_text,
            sizeof(pending_text),
            "%.2f",
            pending_fee
        );


        gtk_list_store_append(
            student_store,
            &iter
        );


        gtk_list_store_set(
            student_store,
            &iter,

            0,
            (const char *)student_id,

            1,
            (const char *)name,

            2,
            (const char *)branch,

            3,
            (const char *)year,

            4,
            (const char *)scholarship,

            5,
            total_text,

            6,
            paid_text,

            7,
            pending_text,

            -1
        );
    }


    sqlite3_finalize(stmt);
}


/* =========================================================
   TREE VIEW SELECTION
   ========================================================= */

void on_tree_selection_changed(
    GtkTreeSelection *selection,
    gpointer data
)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    gchar *student_id = NULL;
    gchar *name = NULL;
    gchar *branch = NULL;
    gchar *year = NULL;
    gchar *scholarship = NULL;
    gchar *total_fee = NULL;
    gchar *paid_fee = NULL;
    gchar *pending_fee = NULL;


    if (gtk_tree_selection_get_selected(
            selection,
            &model,
            &iter
        ))
    {
        gtk_tree_model_get(
            model,
            &iter,

            0,
            &student_id,

            1,
            &name,

            2,
            &branch,

            3,
            &year,

            4,
            &scholarship,

            5,
            &total_fee,

            6,
            &paid_fee,

            7,
            &pending_fee,

            -1
        );


        if (student_id != NULL)
        {
            gtk_entry_set_text(
                GTK_ENTRY(entry_student_id),
                student_id
            );
        }

        if (name != NULL)
        {
            gtk_entry_set_text(
                GTK_ENTRY(entry_name),
                name
            );
        }

        if (branch != NULL)
        {
            gtk_entry_set_text(
                GTK_ENTRY(entry_branch),
                branch
            );
        }

        if (year != NULL)
        {
            gtk_entry_set_text(
                GTK_ENTRY(entry_year),
                year
            );
        }

        if (scholarship != NULL)
        {
            set_combo_scholarship(
                scholarship
            );
        }

        if (total_fee != NULL)
        {
            gtk_entry_set_text(
                GTK_ENTRY(entry_total_fee),
                total_fee
            );
        }

        if (paid_fee != NULL)
        {
            gtk_entry_set_text(
                GTK_ENTRY(entry_paid_fee),
                paid_fee
            );
        }


        /* Free GTK allocated strings */

        g_free(student_id);
        g_free(name);
        g_free(branch);
        g_free(year);
        g_free(scholarship);
        g_free(total_fee);
        g_free(paid_fee);
        g_free(pending_fee);
    }
}


/* =========================================================
   MAIN
   ========================================================= */

int main(int argc, char *argv[])
{
    /*
     * Initialize GTK.
     */
    gtk_init(&argc, &argv);


    /*
     * Apply CSS.
     */
    apply_css();


    /*
     * Open SQLite database.
     */
    open_database();


    /*
     * Create tables if they don't exist.
     */
    create_tables();


    /*
     * Create default admin:
     *
     * username: admin
     * password: admin123
     */
    create_default_admin();


    /*
     * Show login window.
     */
    show_login_window();


    /*
     * Start GTK event loop.
     */
    gtk_main();


    /*
     * Close database after application exits.
     */
    if (db != NULL)
    {
        sqlite3_close(db);
        db = NULL;
    }


    return 0;
}