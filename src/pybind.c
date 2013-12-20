#include "angband.h"
#include "dungeon.h"
#include "files.h"
#include "init.h"
#include "savefile.h"

/* locale junk */
#include "locale.h"
#include "langinfo.h"

#include <Python.h>

#include "main.h"
#include "textui.h"
#include "init.h"

/*
 * List of the available modules in the order they are tried.
 */
static const struct module modules[] =
{
#ifdef USE_X11
	{ "x11", help_x11, init_x11 },
#endif /* USE_X11 */

#ifdef USE_SDL
	{ "sdl", help_sdl, init_sdl },
#endif /* USE_SDL */

#ifdef USE_GCU
	{ "gcu", help_gcu, init_gcu },
#endif /* USE_GCU */

#ifdef USE_TEST
	{ "test", help_test, init_test },
#endif /* !USE_TEST */

#ifdef USE_STATS
	{ "stats", help_stats, init_stats },
#endif /* USE_STATS */
};

static int init_sound_dummy(int argc, char *argv[]) {
	return 0;
}

/*
 * List of sound modules in the order they should be tried.
 */
static const struct module sound_modules[] =
{
#ifdef SOUND_SDL
	{ "sdl", "SDL_mixer sound module", init_sound_sdl },
#endif /* SOUND_SDL */

	{ "none", "No sound", init_sound_dummy },
};

/*
 * A hook for "quit()".
 *
 * Close down, then fall back into "quit()".
 */
static void quit_hook(const char *s)
{
	int j;

	/* Unused parameter */
	(void)s;

	/* Scan windows */
	for (j = ANGBAND_TERM_MAX - 1; j >= 0; j--)
	{
		/* Unused */
		if (!angband_term[j]) continue;

		/* Nuke it */
		term_nuke(angband_term[j]);
	}
}



/*
 * SDL needs a look-in
 */
#ifdef USE_SDL
#include <SDL/SDL.h>
#endif


/*
 * Initialize and verify the file paths, and the score file.
 *
 * Use the ANGBAND_PATH environment var if possible, else use
 * DEFAULT_PATH, and in either case, branch off appropriately.
 *
 * First, we'll look for the ANGBAND_PATH environment variable,
 * and then look for the files in there.  If that doesn't work,
 * we'll try the DEFAULT_PATH constants.  So be sure that one of
 * these two things works...
 *
 * We must ensure that the path ends with "PATH_SEP" if needed,
 * since the "init_file_paths()" function will simply append the
 * relevant "sub-directory names" to the given path.
 *
 * Make sure that the path doesn't overflow the buffer.  We have
 * to leave enough space for the path separator, directory, and
 * filenames.
 */
static void init_stuff(void)
{
	char configpath[512];
	char libpath[512];
	char datapath[512];

	/* Use the angband_path, or a default */
	my_strcpy(configpath, DEFAULT_CONFIG_PATH, sizeof(configpath));
	my_strcpy(libpath, DEFAULT_LIB_PATH, sizeof(libpath));
	my_strcpy(datapath, DEFAULT_DATA_PATH, sizeof(datapath));

	/* Make sure they're terminated */
	configpath[511] = '\0';
	libpath[511] = '\0';
	datapath[511] = '\0';

	/* Hack -- Add a path separator (only if needed) */
	if (!suffix(configpath, PATH_SEP)) my_strcat(configpath, PATH_SEP, sizeof(configpath));
	if (!suffix(libpath, PATH_SEP)) my_strcat(libpath, PATH_SEP, sizeof(libpath));
	if (!suffix(datapath, PATH_SEP)) my_strcat(datapath, PATH_SEP, sizeof(datapath));

	/* Initialize */
	init_file_paths(configpath, libpath, datapath);
}


static const struct {
	const char *name;
	char **path;
	bool setgid_ok;
} change_path_values[] = {
	{ "apex", &ANGBAND_DIR_APEX, TRUE },
	{ "edit", &ANGBAND_DIR_EDIT, FALSE },
	{ "file", &ANGBAND_DIR_FILE, FALSE },
	{ "help", &ANGBAND_DIR_HELP, TRUE },
	{ "info", &ANGBAND_DIR_INFO, TRUE },
	{ "pref", &ANGBAND_DIR_PREF, TRUE },
	{ "xtra", &ANGBAND_DIR_XTRA, TRUE },
	{ "user", &ANGBAND_DIR_USER, TRUE },
	{ "save", &ANGBAND_DIR_SAVE, FALSE },
};

/*
 * Handle a "-d<dir>=<path>" option.
 *
 * Sets any of angband's special directories to <path>.
 *
 * The "<path>" can be any legal path for the given system, and should
 * not end in any special path separator (i.e. "/tmp" or "~/.ang-info").
 */
static void change_path(const char *info)
{
	char *info_copy = NULL;
	char *path = NULL;
	char *dir = NULL;
	unsigned int i = 0;
	char dirpath[512];

	if (!info || !info[0])
		quit_fmt("Try '-d<dir>=<path>'.", info);

	info_copy = string_make(info);
	path = strtok(info_copy, "=");
	dir = strtok(NULL, "=");

	for (i = 0; i < N_ELEMENTS(change_path_values); i++) {
		if (my_stricmp(path, change_path_values[i].name) == 0) {
#ifdef SETGID
			if (!change_path_values[i].setgid_ok)
				quit_fmt("Can't redefine path to %s dir on multiuser setup", path);
#endif

			string_free(*change_path_values[i].path);
			*change_path_values[i].path = string_make(dir);

			/* the directory may not exist and may need to be created. */
			path_build(dirpath, sizeof(dirpath), dir, "");
			if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);
			return;
		}
	}

	quit_fmt("Unrecognised -d paramater %s", path);
}




#ifdef UNIX

/*
 * Find a default user name from the system.
 */
static void user_name(char *buf, size_t len, int id)
{
	struct passwd *pw = getpwuid(id);

	/* Default to PLAYER */
	if (!pw || !pw->pw_name || !pw->pw_name[0] ) {
		my_strcpy(buf, "PLAYER", len);
		return;
	}

	/* Copy and capitalise */
	my_strcpy(buf, pw->pw_name, len);
	my_strcap(buf);
}

#endif /* UNIX */


/**
 * List all savefiles this player can access.
 */
static void list_saves(void)
{
	char fname[256];
	ang_dir *d = my_dopen(ANGBAND_DIR_SAVE);

#ifdef SETGID
	char uid[10];
	strnfmt(uid, sizeof(uid), "%d.", player_uid);
#endif

	if (!d) quit_fmt("Can't open savefile directory");

	printf("Savefiles you can use are:\n");

	while (my_dread(d, fname, sizeof fname)) {
		char path[1024];
		const char *desc;

#ifdef SETGID
		/* Check that the savefile name begins with the user'd ID */
		if (strncmp(fname, uid, strlen(uid)))
			continue;
#endif

		path_build(path, sizeof path, ANGBAND_DIR_SAVE, fname);
		desc = savefile_get_description(path);

		if (desc)
			printf(" %-15s  %s\n", fname, desc);
		else
			printf(" %-15s\n", fname);
	}

	my_dclose(d);

	printf("\nUse angband -u<name> to use savefile <name>.\n");
}


#ifndef SETGID

/*
 * Transition non-setgid installs away from using uid.name style savefile
 * names.
 */
static void transition_savefile_names(void)
{
	char fname[256];
	char uid[10];

	ang_dir *d = my_dopen(ANGBAND_DIR_SAVE);
	if (!d) return;

	strnfmt(uid, sizeof(uid), "%d.", player_uid);

	while (my_dread(d, fname, sizeof fname)) {
		char *newname = fname+strlen(uid);
		char oldpath[1024];
		char newpath[1024];

		/* Check that the savefile name begins with the user'd ID */
		if (strncmp(fname, uid, strlen(uid)) != 0)
			continue;

		/* Sanity check - can't rename "1000." to "" */
		if (!newname[0])
			continue;

		/* Move the file */
		path_build(oldpath, sizeof oldpath, ANGBAND_DIR_SAVE, fname);
		path_build(newpath, sizeof newpath, ANGBAND_DIR_SAVE, newname);

		printf("Moving %s to %s\n", oldpath, newpath);
		file_move(oldpath, newpath);
	}

	my_dclose(d);
}

#endif /* SETGID */



static bool new_game;

/*
 * Pass the appropriate "Initialisation screen" command to the game,
 * getting user input if needed.
 */
static errr get_init_cmd(void)
{
	/* Wait for response */
	pause_line(Term);

	if (new_game)
		cmd_insert(CMD_NEWGAME);
	else
		/* This might be modified to supply the filename in future. */
		cmd_insert(CMD_LOADFILE);

	/* Everything's OK. */
	return 0;
}

/* Command dispatcher for curses, etc builds */
static errr default_get_cmd(cmd_context context, bool wait)
{
	if (context == CMD_INIT)
		return get_init_cmd();
	else
		return textui_get_cmd(context, wait);
}

static void debug_opt(const char *arg) {
	if (streq(arg, "mem-poison-alloc"))
		mem_flags |= MEM_POISON_ALLOC;
	else if (streq(arg, "mem-poison-free"))
		mem_flags |= MEM_POISON_FREE;
	else {
		puts("Debug flags:");
		puts("  mem-poison-alloc: Poison all memory allocations");
		puts("   mem-poison-free: Poison all freed memory");
		exit(0);
	}
}

//--------------------------------------------------------------------------------*
static PyObject*
AngbandPy_run(PyObject *self, PyObject *args)
{
	int i;

	bool done = FALSE;

	const char *mstr = NULL;
	const char *soundstr = NULL;

#ifdef UNIX

	/* Default permissions on files */
	(void)umask(022);

	/* Get the user id */
	player_uid = getuid();

#endif /* UNIX */

#ifdef SETGID

	/* Save the effective GID for later recall */
	player_egid = getegid();

#endif /* UNIX */

	/* Drop permissions */
	safe_setuid_drop();

	/* Get the file paths */
	/* paths may be overriden by -d options, so this has to occur *before*
	   processing command line args */
	init_stuff();

#ifndef SETGID
	/* Transition from 3.4 to 3.5 - rename savefiles with uid at the beginning */
	transition_savefile_names();
#endif

	/* Install "quit" hook */
	quit_aux = quit_hook;

	/* If we were told which mode to use, then use it */
	if (mstr)
		ANGBAND_SYS = mstr;

	if (setlocale(LC_CTYPE, "")) {
		/* Require UTF-8 */
		if (strcmp(nl_langinfo(CODESET), "UTF-8") != 0)
			quit("Angband requires UTF-8 support");
	}

	/* Try the modules in the order specified by modules[] */
	for (i = 0; i < (int)N_ELEMENTS(modules); i++)
	{
		/* User requested a specific module? */
		if (!mstr || (streq(mstr, modules[i].name)))
		{
			ANGBAND_SYS = modules[i].name;
			if (0 == modules[i].init(0, NULL)) //argc, argv))
			{
				done = TRUE;
				break;
			}
		}
	}

	/* Make sure we have a display! */
	if (!done) quit("Unable to prepare any 'display module'!");

#ifdef UNIX

	/* Get the "user name" as a default player name, unless set with -u switch */
	if (!op_ptr->full_name[0]) {
		user_name(op_ptr->full_name, sizeof(op_ptr->full_name), player_uid);

		/* Set the savefile to load */
		savefile_set_name(player_safe_name(p_ptr, FALSE));
	}

	/* Create any missing directories */
	create_needed_dirs();

#endif /* UNIX */

	/* Try the modules in the order specified by sound_modules[] */
	for (i = 0; i < (int)N_ELEMENTS(sound_modules); i++)
		if (!soundstr || streq(soundstr, sound_modules[i].name))
			if (0 == sound_modules[i].init(0, NULL)) //argc, argv))
				break;

	/* Catch nasty signals */
	signals_init();

	/* Set up the command hook */
	cmd_get_hook = default_get_cmd;

	/* Set up the display handlers and things. */
	init_display();

	/* Play the game */
	play_game();

	/* Free resources */
	cleanup_angband();

	/* Quit */
	quit(NULL);

	Py_RETURN_NONE;
}
/*-----------------------------------------------------------------------------*/
PyDoc_STRVAR(_fixme__doc__, "FIXME");
static PyMethodDef AngbandPy_methods[] = {
	{"run", (PyCFunction)AngbandPy_run, METH_VARARGS, _fixme__doc__},
	{NULL}  // Sentinel
};
/*
 * Module initialization below
 * */
struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif


#if PY_MAJOR_VERSION >= 3
static int AngbandPy_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int AngbandPy_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "angband",
        NULL,
        sizeof(struct module_state),
        AngbandPy_methods,
        NULL,
        AngbandPy_traverse,
        AngbandPy_clear,
        NULL
};

#define INITERROR return NULL

PyObject *
PyInit_angband(void)
#else
#define INITERROR return

void
init_angband(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
    PyObject *module = PyModule_Create(&moduledef);
#else
	PyObject *module = Py_InitModule("angband", GLPY_methods);
#endif

    if (module == NULL)
        INITERROR;
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException("angband.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
} /* END Module initialization */
