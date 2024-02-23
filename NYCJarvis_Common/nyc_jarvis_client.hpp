#ifndef NYC_JARVIS_CLIENT_H_
#define NYC_JARVIS_CLIENT_H_

#include "nyc_jarvis_base.hpp"
#include "nyc_jarvis_info.hpp"
#include "nyc_jarvis_threadsafeQueue.hpp"
#include "nyc_jarvis_connection.hpp"
#include "nyc_jarvis_custom_info_types.hpp"
#include<gtk/gtk.h>
namespace nyc
{
    namespace jarvis
    {

        template <typename T>
        class client_interface;

        template <typename T>
        struct Dummy_Class
        {
            GtkTextView* chat_input_text;
            client_interface<T>* c_i;
        };

        template <typename T>
        class client_interface
        {
            public:
                client_interface()
                {
                    
                }

                virtual ~client_interface()
                {
                    Disconnect();
                }

                bool Connect(const std::string& host, const uint16_t port)
                {
                    try
                    {
                        asio::ip::tcp::resolver resolver(m_context);
                        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

                        m_connection = std::make_unique<Connection<T>>(Connection<T>::owner::client, m_context, asio::ip::tcp::socket(m_context), m_qInfosIn);

                        m_connection->ConnectToServer(endpoints);

                        thrContext = std::thread([this]() {m_context.run(); });
                    }
                    catch (std::exception& e)
                    {
                        std::cerr << "Client Exception: " << e.what() << std::endl;
                        return false;
                    }
                    return true;
                }

                void Disconnect()
                {
                    if (IsConnected())
                        m_connection->Disconnect();

                    m_context.stop();

                    if (thrContext.joinable())
                        thrContext.join();

                    m_connection.release();
                }

                bool IsConnected()
                {
                    if (m_connection)
                        return m_connection->IsConnected();
                    else
                        return false;
                }

                threadsafeQueue<owned_info<T>>& Incoming()
                {
                    return m_qInfosIn;
                }

                void Send(const info<T>& info)
                {
                    std::cout<<"Send called\n";
                    if (IsConnected())
                        m_connection->Send(info);
                }

                static void hide_pages_pane(GtkWidget* toggle_pages_pane_btn, GtkPaned* pane)
                {
                    gboolean visible = gtk_paned_get_start_child(pane) != nullptr;

                    if(visible)
                    {
                        gtk_paned_set_start_child(pane, nullptr);
                    }
                    else
                    {

                        GtkWidget* pages_pane_box  = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); 
                        GtkWidget* chat_page_btn = GTK_WIDGET(gtk_button_new_with_label("Chat Page"));
                        GtkWidget* datasets_page_btn = GTK_WIDGET(gtk_button_new_with_label("Datasets Page"));
                        gtk_box_append(GTK_BOX(pages_pane_box), chat_page_btn);
                        gtk_box_append(GTK_BOX(pages_pane_box), datasets_page_btn);
                        gtk_paned_set_start_child(pane, GTK_WIDGET(pages_pane_box));
                    }
                }

                static void send_user_input(GtkButton* send_btn, gpointer d_c)
                {
                    std::cout<<"button clicked\n";
                    Dummy_Class<CustomInfoTypes>* m_d_c = (Dummy_Class<CustomInfoTypes>*)d_c;
                    std::cout<<"m_d_c->chat_input_text: "<<m_d_c->chat_input_text<<std::endl;
                    std::cout<<"m_d_c->chat_input_text==nullptr?: "<<(m_d_c->chat_input_text==nullptr)<<std::endl;
                    std::cout<<"m_d_c->c_i: "<<m_d_c->c_i<<std::endl;
                    GtkTextBuffer* input_text_buffer = gtk_text_view_get_buffer(m_d_c->chat_input_text);
                    
                    GtkTextIter startIterator, endIterator;

                    gtk_text_buffer_get_start_iter(input_text_buffer, &startIterator);
                    gtk_text_buffer_get_end_iter(input_text_buffer, &endIterator);

                    char* input_text = gtk_text_buffer_get_text(input_text_buffer, &startIterator, &endIterator, FALSE);

                    std::cout<<"input_text: "<<input_text<<std::endl;

                    info<CustomInfoTypes> inf;
                    inf.header.id = CustomInfoTypes::I_S;
                    std::string input_text_s(input_text);

                    for(int i=input_text_s.size()-1; i>=0; i--)
                    {
                        inf<<input_text_s[i];
                    } 

                    m_d_c->c_i->Send(inf);

                }
                static void
                activate (GtkApplication* app,
                        gpointer        user_data)
                {
                    client_interface<T>* c_i = (client_interface<T>*) user_data;

                    GtkWidget *window;
                    
                    //GTK Window Custom Header
                    window = gtk_application_window_new (app);
                    // GtkBuilder *builder = gtk_builder_new_from_file("C:/Users/tahfi/VSCodeProjects/NYCJarvis/NYCJarvis_Common/GTK_UI_Files/header_bar.ui");
                    // GtkHeaderBar* customHeader = GTK_HEADER_BAR(gtk_builder_get_object(builder, "custom_header"));
                    // GtkTextView* title = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "header_text_view"));
                    // GtkButton* button_1 = GTK_BUTTON(gtk_builder_get_object(builder, "header_btn_1"));
                    // GtkButton* button_2 = GTK_BUTTON(gtk_builder_get_object(builder, "header_btn_2"));
                    // gtk_text_buffer_set_text(gtk_text_view_get_buffer(title), "hello world", -1);
                    // gtk_button_set_label(button_1, "Click Me 1!");
                    // gtk_button_set_label(button_2, "Click Me 2!");
                    // gtk_widget_set_size_request(GTK_WIDGET(title), 40, 50);
                    
                    // gtk_widget_set_size_request(GTK_WIDGET(button_1), 40, 50);
                    // gtk_widget_set_size_request(GTK_WIDGET(button_2), 40, 50);
                    // gtk_window_set_titlebar(GTK_WINDOW (window), GTK_WIDGET(customHeader));

                    // gtk_window_set_title (GTK_WINDOW (window), "NYCJarvis");

                    GtkBuilder* builder = gtk_builder_new_from_file("C:/Users/tahfi/VSCodeProjects/NYCJarvis/NYCJarvis_Common/GTK_UI_Files/chat_page.ui");
                    GtkTextView* chat_input_text = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "chat_input_text"));
                    GtkPaned* pane = GTK_PANED(gtk_builder_get_object(builder, "pane"));
                    GtkButton* toggle_pages_pane_btn = GTK_BUTTON(gtk_builder_get_object(builder, "toggle_pages_pane"));

                    GtkButton* send_btn = GTK_BUTTON(gtk_builder_get_object(builder, "chat_input_btn"));

                    g_signal_connect(toggle_pages_pane_btn, "clicked", G_CALLBACK(c_i->hide_pages_pane), pane);
                    
                    g_signal_emit_by_name (toggle_pages_pane_btn, "clicked");
                    
                    Dummy_Class<CustomInfoTypes> d_c;
                    d_c.chat_input_text = chat_input_text;
                    d_c.c_i = c_i;
                    std::cout<<"chat_input_text: "<<chat_input_text<<std::endl;
                    std::cout<<"d_c.chat_input_text: "<<d_c.chat_input_text<<std::endl;
                    std::cout<<"c_i: "<<c_i<<std::endl;
                    std::cout<<"d_c.c_i: "<<d_c.c_i<<std::endl;
                    g_signal_connect(send_btn, "clicked", G_CALLBACK(c_i->send_user_input), &d_c);
                    gtk_text_view_set_wrap_mode(chat_input_text, GTK_WRAP_CHAR);
                    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
                    GtkHeaderBar* customHeader = GTK_HEADER_BAR(gtk_builder_get_object(builder, "header_bar"));
                    gtk_window_set_titlebar(GTK_WINDOW (window), GTK_WIDGET(customHeader));
                   
                    gtk_window_present (GTK_WINDOW (window));

                    while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
                        g_main_context_iteration (NULL, TRUE);

                }

                void instantiateGTKApp()
                {
                    app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
                }

                void conntectAndRunGTKAPP()
                {
                    int status;
                    std::cout<<"this connectAndRunGTKAPP(): "<<this<<std::endl;
                    g_signal_connect (app, "activate", G_CALLBACK (activate), this);
                    status = g_application_run (G_APPLICATION (app), 0, NULL);
                    std::cout<<"reached the end\n";
                }
                void StartGTK()
                {
                    

                    instantiateGTKApp();
                    conntectAndRunGTKAPP();
                    
                    
                    g_object_unref (app);
                }

                protected:
                    asio::io_context m_context;
                    std::thread thrContext;

                    std::unique_ptr<Connection<T>> m_connection;

                private:
                    threadsafeQueue<owned_info<T>> m_qInfosIn;
                    GtkApplication *app;

        };
    }
}

#endif