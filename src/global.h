#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifdef _GLOBAL
#define global 
#else 
#define global extern
#endif


global GtkWidget *main_window;
global GtkWidget *hidden_entry;
global GtkWidget *word_entry;
global GtkWidget *combo_method;
global GtkWidget *combo_word;
global GtkWidget *button_mode;
global GtkWidget *button_start;
global GtkWidget *button_auto;
global GtkWidget *button_popup;
global GtkWidget *button_back;
global GtkWidget *button_forward;
global GtkWidget *status_bar;

global GtkTooltips *tooltip;


global GList *search_result;
global RESULT *current_result;

global GList *ending_list;
global GList *ending_list_ja;

global GdkFont *font_normal;
global GdkFont *font_bold;
global GdkFont *font_superscript;
global GdkFont *font_italic;

global gint font_height;
global gint font_width;
global gint font_ascent;
global gint font_descent;

global gchar *fs_codeset;

global struct _search_method search_method[64];	

global gint bstarting_up;
global gint max_search;
global gint max_heading;
global gint max_remember_words;
global gint dict_button_length;
global gint auto_interval;
global gint auto_minchar;
global gint auto_maxchar;
global gint bshow_menu_bar;
global gint bshow_status_bar;
global gint bshow_dict_bar;
global gint bshow_tree_tab;
global gint bending_only_nohit;
global gint bending_correction;
global gint bshow_popup_title;
global gint bbeep_on_nohit;
global gint bignore_locks;
global gint popup_width;
global gint popup_height;
global gchar *wave_template;
global gchar *mpeg_template;
global gchar *browser_template;
global gchar *open_template;
global gint bbrowser_external;
global gint buse_http_proxy;
global gint window_x, window_y;
global gint window_width, window_height;
global gint tree_width, tree_height;
global gint bsmooth_scroll;
global gint scroll_step;
global gint scroll_time;
global gint scroll_margin;
global gint bsort_by_dictionary;
global gint pane_direction;
global gint tab_position;
global gint bignore_case;
global gint bsuppress_hidden_files;
global gint bemphasize_keyword;
global gint bshow_image;
global gint bshow_splash;
global gint bword_search_automatic;
global gint additional_lines;
global gint additional_chars;
global gint cache_size;
global gint max_bytes_to_guess;
global gint bshow_filename;
global gint bheading_auto_calc;
global gint benable_button_color;
global gint selection_mode;
global gint bplay_sound_internally;



global gint line_space;
global gint h_space;
global gint v_space;
global gint h_border;
global gint v_border;
global gint gaiji_adjustment;


global gchar *user_dir;
global gchar *temp_dir;
global gchar *package_dir;
global gchar *cache_dir;

global gchar *fontset_normal;
global gchar *fontset_bold;
global gchar *fontset_italic;
global gchar *fontset_subscript;
global gchar *fontset_superscript;

global gchar *color_str[NUM_COLORS];
global GdkColor colors[NUM_COLORS];

global GtkTreeStore *web_store;
global GtkTreeStore *dict_store;
global GtkListStore *stemming_en_store;
global GtkListStore *stemming_ja_store;
global GtkListStore *shortcut_store;
global GtkListStore *filter_store;
global GtkListStore *dirgroup_store;

global GtkTextTag *tag_keyword;
global GtkTextTag *tag_bold;
global GtkTextTag *tag_link;
global GtkTextTag *tag_sound;
global GtkTextTag *tag_movie;
global GtkTextTag *tag_italic;
global GtkTextTag *tag_superscript;
global GtkTextTag *tag_subscript;
global GtkTextTag *tag_center;
global GtkTextTag *tag_plain;
global GtkTextTag *tag_gaiji;
global GtkTextTag *tag_colored;
global GtkTextTag *tag_reverse;
global GtkTextTag *tag_indent[MAX_INDENT];

#endif /* __GLOBAL_H__ */
