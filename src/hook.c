/*  Copyright (C) 2001-2003  Kenichi Suto
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "defs.h"
#include "global.h"

#include "eb.h"

EB_Hookset text_hookset;
EB_Hookset heading_hookset;
EB_Hookset candidate_hookset;


static EB_Error_Code hook_nop(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_initialize(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_narrow_font(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_wide_font(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_indent(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_newline(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_no_newline(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_reference(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_candidate(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_narrow(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_superscript(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_subscript(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_emphasis(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_keyword(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_modification(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_euc_to_ascii(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_color(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_mono(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_gray(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_wave(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_mpeg(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Error_Code hook_graphic_reference(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv);

static EB_Hook text_hooks[] = {
//  {EB_HOOK_INITIALIZE,             hook_initialize},
//  {EB_HOOK_STOP_CODE,              eb_hook_stop_code},
//  {EB_HOOK_STOP_CODE,              hook_stopcode},
  {EB_HOOK_SET_INDENT,                hook_indent},
  {EB_HOOK_NEWLINE,                hook_newline},
  {EB_HOOK_NARROW_FONT,            hook_narrow_font},
  {EB_HOOK_WIDE_FONT,	           hook_wide_font},
  {EB_HOOK_BEGIN_REFERENCE,        hook_reference},
  {EB_HOOK_END_REFERENCE,          hook_reference},
  {EB_HOOK_BEGIN_CANDIDATE,        hook_candidate},
  {EB_HOOK_END_CANDIDATE_LEAF,     hook_candidate},
  {EB_HOOK_END_CANDIDATE_GROUP,    hook_candidate},
//  {EB_HOOK_NARROW_JISX0208,        hook_euc_to_ascii},
  {EB_HOOK_BEGIN_COLOR_BMP,        hook_color},
  {EB_HOOK_BEGIN_COLOR_JPEG,       hook_color},
  {EB_HOOK_END_COLOR_GRAPHIC,      hook_color},
  {EB_HOOK_BEGIN_MONO_GRAPHIC,     hook_mono},
  {EB_HOOK_END_MONO_GRAPHIC,       hook_mono},
  {EB_HOOK_BEGIN_GRAY_GRAPHIC,     hook_gray},
  {EB_HOOK_END_GRAY_GRAPHIC,       hook_gray},
  {EB_HOOK_BEGIN_WAVE,             hook_wave},
  {EB_HOOK_END_WAVE,               hook_wave},
  {EB_HOOK_BEGIN_MPEG,             hook_mpeg},
  {EB_HOOK_END_MPEG,               hook_mpeg},
//  {EB_HOOK_BEGIN_GRAPHIC_REFERENCE,hook_graphic_reference},
//  {EB_HOOK_END_GRAPHIC_REFERENCE,  hook_graphic_reference},
//  {EB_HOOK_GRAPHIC_REFERENCE,      hook_graphic_reference},
//  {EB_HOOK_BEGIN_NO_NEWLINE,       hook_no_newline},
//  {EB_HOOK_END_NO_NEWLINE,         hook_no_newline},
//  {EB_HOOK_BEGIN_NARROW,           hook_narrow},
//  {EB_HOOK_END_NARROW,             hook_narrow},
  {EB_HOOK_BEGIN_SUPERSCRIPT,      hook_superscript},
  {EB_HOOK_END_SUPERSCRIPT,        hook_superscript},
  {EB_HOOK_BEGIN_SUBSCRIPT,        hook_subscript},
  {EB_HOOK_END_SUBSCRIPT,          hook_subscript},
  {EB_HOOK_BEGIN_EMPHASIS,         hook_emphasis},
  {EB_HOOK_END_EMPHASIS,           hook_emphasis},
  {EB_HOOK_BEGIN_KEYWORD,          hook_keyword},
  {EB_HOOK_END_KEYWORD,            hook_keyword},
#ifdef EB_HOOK_BEGIN_DECORATION
  {EB_HOOK_BEGIN_DECORATION,       hook_modification},
#endif
#ifdef EB_HOOK_END_DECORATION
  {EB_HOOK_END_DECORATION,         hook_modification},
#endif
  {EB_HOOK_NULL, NULL},
};


static EB_Hook heading_hooks[] = {
  {EB_HOOK_NEWLINE,                hook_newline},
  {EB_HOOK_NARROW_FONT,            hook_narrow_font},
  {EB_HOOK_WIDE_FONT,	           hook_wide_font},
//  {EB_HOOK_BEGIN_CANDIDATE,        hook_candidate},
//  {EB_HOOK_END_CANDIDATE_LEAF,     hook_candidate},
//  {EB_HOOK_END_CANDIDATE_GROUP,    hook_candidate},
  {EB_HOOK_NARROW_JISX0208,        eb_hook_euc_to_ascii},
  {EB_HOOK_BEGIN_SUPERSCRIPT,      hook_superscript},
  {EB_HOOK_END_SUPERSCRIPT,        hook_superscript},
  {EB_HOOK_BEGIN_SUBSCRIPT,        hook_subscript},
  {EB_HOOK_END_SUBSCRIPT,          hook_subscript},
  {EB_HOOK_NULL, NULL},
};

static EB_Hook candidate_hooks[] = {
  {EB_HOOK_NARROW_FONT,            hook_narrow_font},
  {EB_HOOK_WIDE_FONT,	           hook_wide_font},
  {EB_HOOK_BEGIN_CANDIDATE,        hook_candidate},
  {EB_HOOK_END_CANDIDATE_LEAF,     hook_candidate},
  {EB_HOOK_END_CANDIDATE_GROUP,    hook_candidate},
  {EB_HOOK_NULL, NULL},
};

gint hook_level = -1;
EB_Hook_Code hook_stack[256];

static void push_hook_stack(EB_Hook_Code code)
{
	if(hook_level >= 255) {
		// Nest too deep
		return;
	}
	hook_level ++;
	hook_stack[hook_level] = code;
}

static gboolean check_hook_stack(EB_Hook_Code code)
{
	if(hook_level >= 255) {
		// Nest too deep
		return(TRUE);
	}
	if(hook_stack[hook_level] == code)
		return(TRUE);

	LOG(LOG_INFO, "hook_stack unmatch : %d != %d", hook_stack[hook_level], code);
	return(FALSE);
}

static void pop_hook_stack()
{
	if(hook_level == -1)
		return;

	hook_level --;
}

static EB_Error_Code hook_nop(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	return EB_SUCCESS;
}

static EB_Error_Code hook_initialize(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	return EB_SUCCESS;
}

static EB_Error_Code hook_narrow_font(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	
	char text[64];

	sprintf (text, "<gaiji code=h%04x>", argv[0]);
	eb_write_text_string(book, text);

	return EB_SUCCESS;
}

static EB_Error_Code hook_wide_font(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	char text[64];

	sprintf (text, "<gaiji code=z%04x>", argv[0]);
	eb_write_text_string(book, text);

	return EB_SUCCESS;
}

static EB_Error_Code hook_indent(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	char text[64];

	sprintf (text, "<indent position=%d>", argv[1]);
	eb_write_text_string(book, text);
/*
	for(i = 0 ; i < argv[1] ; i ++){
		eb_write_text_string(book, "  ");
	}
*/
	return EB_SUCCESS;
}

static EB_Error_Code hook_newline(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{

        eb_write_text_string(book, "\n");
	return EB_SUCCESS;
}


static EB_Error_Code hook_narrow(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	switch (code) {
	case EB_HOOK_BEGIN_NARROW:
//		eb_write_text_string(book, "<narrow>");
		break;
	case EB_HOOK_END_NARROW:
//		eb_write_text_string(book, "</narrow>");
		break;
	}
	return EB_SUCCESS;

}

static EB_Error_Code hook_no_newline(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	switch (code) {
	case EB_HOOK_BEGIN_NO_NEWLINE:
//		eb_write_text_string(book, "<nonewline>");
		break;
	case EB_HOOK_END_NO_NEWLINE:
//		eb_write_text_string(book, "</nonewline>");
		break;
	}
	return EB_SUCCESS;

}

static EB_Error_Code hook_reference(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	gchar text[64];


	switch (code) {
	case EB_HOOK_BEGIN_REFERENCE:
		sprintf (text, "<reference>");
		eb_write_text_string(book, text);
		break;
	case EB_HOOK_END_REFERENCE:
		sprintf (text, "</reference page=0x%x offset=0x%x>", argv[1], argv[2]);
		eb_write_text_string(book, text);
		break;
	}
	return EB_SUCCESS;

}

static EB_Error_Code hook_candidate(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	gchar text[64];


	switch (code) {
	case EB_HOOK_BEGIN_CANDIDATE:
		eb_write_text_string(book, "<candidate>");
		break;
	case EB_HOOK_END_CANDIDATE_LEAF:
		sprintf (text, "</candidate>");
		eb_write_text_string(book, text);
		break;
	case EB_HOOK_END_CANDIDATE_GROUP:
		sprintf (text, "</candidate page=0x%x offset=0x%x>", argv[1], argv[2]);
		eb_write_text_string(book, text);
		break;
	}
	return EB_SUCCESS;

}

static EB_Error_Code hook_superscript(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	gchar text[64];


	switch (code) {
	case EB_HOOK_BEGIN_SUPERSCRIPT:
		eb_write_text_string(book, "<sup>");
		push_hook_stack(code);
		break;
	case EB_HOOK_END_SUPERSCRIPT:
		if(check_hook_stack(EB_HOOK_BEGIN_SUPERSCRIPT) == TRUE) {
			sprintf (text, "</sup>");
			eb_write_text_string(book, text);
			pop_hook_stack();
		}
		break;
	}
	return EB_SUCCESS;
}

static EB_Error_Code hook_subscript(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	gchar text[64];


	switch (code) {
	case EB_HOOK_BEGIN_SUBSCRIPT:
		eb_write_text_string(book, "<sub>");
		push_hook_stack(code);
		break;
	case EB_HOOK_END_SUBSCRIPT:
		if(check_hook_stack(EB_HOOK_BEGIN_SUBSCRIPT) == TRUE) {
			sprintf (text, "</sub>");
			eb_write_text_string(book, text);
			pop_hook_stack();
		}
		break;
	}
	return EB_SUCCESS;
}

static EB_Error_Code hook_emphasis(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	gchar text[64];


	switch (code) {
	case EB_HOOK_BEGIN_EMPHASIS:
		eb_write_text_string(book, "<emphasis>");
		push_hook_stack(code);
		break;
	case EB_HOOK_END_EMPHASIS:
		if(check_hook_stack(EB_HOOK_BEGIN_EMPHASIS) == TRUE) {
			sprintf (text, "</emphasis>");
			eb_write_text_string(book, text);
			pop_hook_stack();
		}
		break;
	}
	return EB_SUCCESS;
}

static EB_Error_Code hook_keyword(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	gchar text[64];


	switch (code) {
	case EB_HOOK_BEGIN_KEYWORD:
		sprintf (text, "<keyword argv1=%x>", argv[1]);
		eb_write_text_string(book, text);
		push_hook_stack(code);
		break;
	case EB_HOOK_END_KEYWORD:
		if(check_hook_stack(EB_HOOK_BEGIN_KEYWORD) == TRUE) {
			sprintf (text, "</keyword>");
			eb_write_text_string(book, text);
			pop_hook_stack();
		}
		break;
	}
	return EB_SUCCESS;
}


static EB_Error_Code hook_modification(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	gchar text[64];

#ifdef EB_HOOK_BEGIN_DECORATION
#ifdef EB_HOOK_END_DECORATION

	switch (code) {
	case EB_HOOK_BEGIN_DECORATION:
		sprintf (text, "<modification method=%d>", argv[1]);
		eb_write_text_string(book, text);
		push_hook_stack(code);
		break;
	case EB_HOOK_END_DECORATION:
		if(check_hook_stack(EB_HOOK_BEGIN_DECORATION) == TRUE) {
			sprintf (text, "</modification>");
			eb_write_text_string(book, text);
			pop_hook_stack();
		}
		break;
	}

#endif
#endif
	return EB_SUCCESS;
}


/*
 * Hook for a reference to color graphic data.
 */
static EB_Error_Code hook_color(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	gchar text[64];
	
	switch (code) {
	case EB_HOOK_BEGIN_COLOR_JPEG:

		sprintf (text, "<jpeg page=0x%x offset=0x%x>", argv[2], argv[3]);
		eb_write_text_string(book, text);
		break;
	case EB_HOOK_BEGIN_COLOR_BMP:
		sprintf (text, "<bmp page=0x%x offset=0x%x>", argv[2], argv[3]);
		eb_write_text_string(book, text);
		break;
/*
	case EB_HOOK_END_COLOR_GRAPHIC:
		sprintf (text, "</jpeg=0x%x:0x%x>", argv[2], argv[3]);
		eb_write_text_string(book, text);
		break;
*/
	}
	return EB_SUCCESS;
}


/*
 * Hook for a reference to MONO graphic data.
 */
static EB_Error_Code hook_mono(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	gchar text[64];

	switch (code) {
	case EB_HOOK_BEGIN_MONO_GRAPHIC:
		// argv[2] : height
		// argv[3] : width

		sprintf (text, "<mono width=%d height=%d>", argv[3], argv[2]);
		eb_write_text_string(book, text);
		break;

	case EB_HOOK_END_MONO_GRAPHIC:
		// argv[1] : block
		// argv[2] : offset

		sprintf (text, "</mono page=0x%x offset=0x%x>", argv[1], argv[2]);
		eb_write_text_string(book, text);
		break;
	}
	return EB_SUCCESS;
}

/*
 * Hook for a reference to GRAY graphic data.
 */
static EB_Error_Code hook_gray(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	gchar text[64];

	switch (code) {
	case EB_HOOK_BEGIN_GRAY_GRAPHIC:

		sprintf (text, "<gray page=0x%x offset=0x%x>", argv[2], argv[3]);
		eb_write_text_string(book, text);
		break;
/*
	case EB_HOOK_END_GRAY_GRAPHIC:
		sprintf (text, "</gray=0x%x:0x%x>", argv[2], argv[3]);
		eb_write_text_string(book, text);
		break;
*/
	}
	return EB_SUCCESS;
}


/*
 * Hook for a reference to WAVE sound data.
 */
static EB_Error_Code hook_wave(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
	off_t start_location;
	off_t end_location;
	size_t data_size;
	gchar text[64];

	/*
	 * Set binary context.
	 */
	start_location = (off_t)(argv[2] - 1) * EB_SIZE_PAGE + argv[3];
	end_location   = (off_t)(argv[4] - 1) * EB_SIZE_PAGE + argv[5];
	data_size = end_location - start_location;

	switch (code) {
	case EB_HOOK_BEGIN_WAVE:
		eb_write_text_string(book, "<wave>");
		break;
	case EB_HOOK_END_WAVE:
		sprintf (text, "</wave page=0x%x offset=0x%x size=%d>", argv[2], argv[3], data_size);
		eb_write_text_string(book, text);
		break;
	}
	return EB_SUCCESS;
}


/*
 * Hook for a reference to MPEG sound data.
 */
static EB_Error_Code hook_mpeg(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
    char file_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];
    char text[256];

	switch (code) {
	case EB_HOOK_BEGIN_MPEG:
		break;
	case EB_HOOK_END_MPEG:
		if (eb_compose_movie_file_name(argv + 2, file_name) != EB_SUCCESS)
			return EB_SUCCESS;
		sprintf(text, "<mpeg filename=%s>", file_name);
		eb_write_text_string(book, text);
		break;
	}
    return EB_SUCCESS;
}


/*
 * Hook for a reference to graphic reference.
 */
static EB_Error_Code hook_graphic_reference(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
    char text[256];

	switch (code) {
	case EB_HOOK_GRAPHIC_REFERENCE:
	case EB_HOOK_BEGIN_GRAPHIC_REFERENCE:
		sprintf (text, "<graphic_reference page=0x%x offset=0x%x>", argv[1], argv[2]);
		eb_write_text_string(book, text);
		break;
	case EB_HOOK_END_GRAPHIC_REFERENCE:
		break;
	}
    return EB_SUCCESS;
}

/*
 * EUC JP to ASCII conversion table.
 */
#define EUC_TO_ASCII_TABLE_START	0xa0
#define EUC_TO_ASCII_TABLE_END		0xff

static const unsigned char euc_a1_to_ascii_table[] = {
    0x00, 0x20, 0x00, 0x00, 0x2c, 0x2e, 0x00, 0x3a,     /* 0xa0 */
    0x3b, 0x3f, 0x21, 0x00, 0x00, 0x00, 0x60, 0x00,     /* 0xa8 */
    0x5e, 0x7e, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xb0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x2f,     /* 0xb8 */
    0x5c, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x27,     /* 0xc0 */
    0x00, 0x22, 0x28, 0x29, 0x00, 0x00, 0x5b, 0x5d,     /* 0xc8 */
    0x7b, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xd0 */
    0x00, 0x00, 0x00, 0x00, 0x2b, 0x2d, 0x00, 0x00,     /* 0xd8 */
    0x00, 0x3d, 0x00, 0x3c, 0x3e, 0x00, 0x00, 0x00,     /* 0xe0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c,     /* 0xe8 */
    0x24, 0x00, 0x00, 0x25, 0x23, 0x26, 0x2a, 0x40,     /* 0xf0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xf8 */
};

static const unsigned char euc_a3_to_ascii_table[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xa0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xa8 */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,     /* 0xb0 */
    0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xb8 */
    0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,     /* 0xc0 */
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,     /* 0xc8 */
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,     /* 0xd0 */
    0x58, 0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xd8 */
    0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,     /* 0xe0 */
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,     /* 0xe8 */
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,     /* 0xf0 */
    0x78, 0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xf8 */
};

/*
 * Latin-1 character to entity reference table.
 * (e.g. 'a  --> &aacute;)
 */
const char *latin1_entity_name_table[] = {
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x00 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x08 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x10 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x18 */
    NULL, NULL, "quot", NULL, NULL, NULL, "amp", NULL,	/* 0x20 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x28 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x30 */
    NULL, NULL, NULL,   NULL, "lt", NULL, "gt",  NULL,	/* 0x38 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x40 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x48 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x50 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x58 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x60 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x68 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x70 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x78 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x80 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x88 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x90 */
    NULL, NULL, NULL,   NULL, NULL, NULL, NULL,  NULL,	/* 0x98 */
    "nbsp",   "iexcl",  "cent",   "pound",	/* 0xa0 */
    "curren", "yen",    "brvbar", "sect",	/* 0xa4 */
    "uml",    "copy",   "ordf",   "laquo",	/* 0xa8 */
    "not",    "shy",    "reg",    "macr",	/* 0xac */
    "deg",    "plusmn", "sup2",   "sup3",	/* 0xb0 */
    "acute",  "micro",  "para",   "middot",	/* 0xb4 */
    "cedil",  "sup1",   "ordm",   "requo",	/* 0xb8 */
    "frac14", "frac12", "farc34", "iquest",	/* 0xbc */
    "Agrave", "Aacute", "Acirc",  "Atilde",	/* 0xc0 */
    "Auml",   "Aring",  "AElig",  "Ccedil",	/* 0xc4 */
    "Egrave", "Eacute", "Ecirc",  "Euml",	/* 0xc8 */
    "Igrave", "Iacute", "Icirc",  "Iuml",	/* 0xcc */
    "ETH",    "Ntilde", "Ograve", "Oacute",	/* 0xd0 */
    "Ocirc",  "Otilde", "Ouml",   "times",	/* 0xd4 */
    "Oslash", "Ugrave", "Uacute", "Ucirc",	/* 0xd8 */
    "Uuml",   "Yacute", "THORN",  "szlig",	/* 0xdc */
    "agrave", "aacute", "acirc",  "atilde",	/* 0xe0 */
    "auml",   "aring",  "aelig",  "ccedil",	/* 0xe4 */
    "egrave", "eacute", "ecirc",  "euml",	/* 0xe8 */
    "igrave", "iacute", "icirc",  "iuml",	/* 0xec */
    "eth",    "ntilde", "ograve", "oacute",	/* 0xf0 */
    "ocirc",  "otilde", "ouml",   "divide",	/* 0xf4 */
    "oslash", "ugrave", "uacute", "ucirc",	/* 0xf8 */
    "uuml",   "yacute", "thorn",  "yuml"	/* 0xfc */
};

/*
 * Hook which converts a character from EUC-JP to ASCII.
 */

static EB_Error_Code hook_euc_to_ascii(EB_Book *book, EB_Appendix *appendix, void *container, EB_Hook_Code code, int argc, const unsigned int *argv)
{
    int in_code1, in_code2;
    int out_code = 0;
    const char *entity;
    
    in_code1 = argv[0] >> 8;
    in_code2 = argv[0] & 0xff;

    if (in_code2 < EUC_TO_ASCII_TABLE_START
	|| EUC_TO_ASCII_TABLE_END < in_code2) {
	out_code = 0;
    } else if (in_code1 == 0xa1) {
	out_code = euc_a1_to_ascii_table[in_code2 - EUC_TO_ASCII_TABLE_START];
    } else if (in_code1 == 0xa3) {
	out_code = euc_a3_to_ascii_table[in_code2 - EUC_TO_ASCII_TABLE_START];
    }

    if (out_code == 0)
	eb_write_text_byte2(book, in_code1, in_code2);
    else {
	entity = latin1_entity_name_table[out_code];
	if (entity != NULL) {
	    eb_write_text_byte1(book, '&');
	    eb_write_text_string(book, entity);
	    eb_write_text_byte1(book, ';');
	} else {
	    eb_write_text_byte1(book, out_code);
	}
    }
	
    return EB_SUCCESS;
}

EB_Error_Code initialize_hooksets()
{

	EB_Error_Code error_code;

	eb_initialize_hookset (&text_hookset);

	error_code = eb_set_hooks (&text_hookset, text_hooks);
	if(error_code != EB_SUCCESS){
		fprintf(stderr, "Failed to set hookset(text) : %s\n",
			 eb_error_message(error_code));
		return(1);
	}


	eb_initialize_hookset (&heading_hookset);

	error_code = eb_set_hooks (&heading_hookset, heading_hooks);
	if(error_code != EB_SUCCESS){
		fprintf(stderr, "Failed to set hookset(heading) : %s\n",
			 eb_error_message(error_code));
		return(1);
	}

	eb_initialize_hookset (&candidate_hookset);

	error_code = eb_set_hooks (&candidate_hookset, candidate_hooks);
	if(error_code != EB_SUCCESS){
		fprintf(stderr, "Failed to set hookset(candidate) : %s\n",
			 eb_error_message(error_code));
		return(1);
	}

	return EB_SUCCESS;
}

void finalize_hooksets()
{
	eb_finalize_hookset (&text_hookset);
	eb_finalize_hookset (&heading_hookset);
	eb_finalize_hookset (&candidate_hookset);
}
