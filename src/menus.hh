
#define new_app_cb NULL
#define open_cd NULL
#define save_as_cb NULL
#define close_cb NULL
#define exit_cb NULL
#define open_cb NULL

const char* stage_name = "Stage";

const char* stage_authors[] = {"Richard Vaughan", "Andrew Howard", "Brian Gerkey"};

const char* stage_copyright = "the Authors, 2000-2002";

const char** stage_documenters = NULL;

const char* stage_translators = NULL;

const char* stage_comments = "Funding & facilites:\n\tDARPA\n"
"\tUniversity of Southern California\n"
"\tHRL Laboratories LLC\n"
"\nHomepage: http://playerstage.sourceforge.net\n"
"\n\"All the World's a stage "
"and all the men and women merely players\"";

GdkPixbuf* stage_logo_pixbuf = NULL;

static GnomeUIInfo file_menu[] = {
  GNOMEUIINFO_MENU_NEW_ITEM(N_("_New Window"),
                            N_("Create a new text viewer window"), 
                            new_app_cb, NULL),
  GNOMEUIINFO_MENU_OPEN_ITEM(open_cb,NULL),
  GNOMEUIINFO_MENU_SAVE_AS_ITEM(save_as_cb,NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_CLOSE_ITEM(close_cb,NULL),
  GNOMEUIINFO_MENU_EXIT_ITEM(exit_cb,NULL),
  GNOMEUIINFO_END
};


static GnomeUIInfo main_menu[] = {
  GNOMEUIINFO_MENU_FILE_TREE(file_menu),
  GNOMEUIINFO_END
};

