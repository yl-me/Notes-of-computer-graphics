#include "FreeType.h"

namespace freetype {
/*
*	OpenGL textures need to have dimensions that are powers of two, 
*  so we need to pad the font bitmaps created by FreeType to make them a legal size
*/
inline int next_p2(int a)
{
	int rval = 1;
	while (rval < a) {
		rval <<= 1;
	}
	return rval;
}
// Create a displsy list corresponding to the given character 
void make_dlist(FT_Face face, char ch, GLuint list_base, GLuint* tex_base)
{
	/* The first thing we do is get FreeType to render our character into a bitmap */

	// Load the glyph for our character
	if (FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_DEFAULT)) {
		throw std::runtime_error("FT_Load_Glyph failed");
	}
	// Move the face's glyph into a glyph bject
	FT_Glyph glyph;
	if (FT_Get_Glyph(face->glyph, &glyph)) {
		throw std::runtime_error("FT_Get_Glyph failed");
	}
	// Convert the glyph to a bitmap
	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

	// This reference will make accessing the bitmap easier
	FT_Bitmap& bitmap = bitmap_glyph->bitmap;

	int width = next_p2(bitmap.width);            // Get the width and heigth
	int height = next_p2(bitmap.rows);

	// Allocate memory
	GLubyte* expanded_data = new GLubyte[2 * width * height];

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			expanded_data[2 * (j + i*width)] = expanded_data[2 * (j + i*width) + 1] =
				(j >= bitmap.width || i >= bitmap.rows) ? 0 : bitmap.buffer[j + i*bitmap.width];
			// Bugfix

			// expanded_data[2 * (j + i * width)] = 255;
			// expanded_data[2 * (j + i * width) + 1] =
			// (j >= bitmap.width || i >= bitmap.rows) ? 0 : bitmap.buffer[j + bitmap.width * i];
		}
	}
	//  The edges of the text will be slightly translucent
	glBindTexture(GL_TEXTURE_2D, tex_base[ch]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Using GL_LUMINANCE_ALPHA to indicate that we are using 2 channel data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_LUMINANCE_ALPHA, 
		GL_UNSIGNED_BYTE, expanded_data);

	delete[] expanded_data;

	// The display list
	glNewList(list_base + ch, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, tex_base[ch]);
	glPushMatrix();

	glTranslatef(bitmap_glyph->left, 0, 0);
	glTranslatef(0, bitmap_glyph->top-bitmap.rows + 4, 0);

	float x = (float)bitmap.width / (float)width;
	float y = (float)bitmap.rows / (float)height;

	glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(0, bitmap.rows);
		glTexCoord2f(0, y); glVertex2f(0, 0);
		glTexCoord2f(x, y); glVertex2f(bitmap.width, 0);
		glTexCoord2f(x, 0); glVertex2f(bitmap.width, bitmap.rows);
	glEnd();

	glPopMatrix();
	glTranslatef(face->glyph->advance.x >> 6, 0, 0);
	// Increment the raster position as if we were a bitmap font
	// glBitmap(0, 0, 0, 0, face->glyph->advance.x >> 6, 0, NULL);

	glEndList();
	// Bugfix
	FT_Done_Glyph(glyph);
}
// Initialize
void font_data::init(const char* fname, unsigned int h)
{
	textures = new GLuint[128];
	this->h = h;
	// Create and initilize a freeType font library
	FT_Library library;
	if (FT_Init_FreeType(&library)) {
		throw std::runtime_error("FT_Init_FreeType failed");
	}

	FT_Face face;        // The object In Which FreeType holds information on a given font
	if (FT_New_Face(library, fname, 0, &face)) {
		throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");
	}

	FT_Set_Char_Size(face, h << 6, h << 6, 96, 96);
	list_base = glGenLists(128);
	glGenTextures(128, textures);

	// Create fonts
	for (unsigned char i = 0; i < 128; ++i) {
		make_dlist(face, i, list_base, textures);
	}
	// Free
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

// Free
void font_data::clean()
{
	glDeleteLists(list_base, 128);
	glDeleteTextures(128, textures);
	delete[] textures;
}

inline void pushScreenCoordinateMatrix()
{
	glPushAttrib(GL_TRANSFORM_BIT);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(viewport[0], viewport[2], viewport[1], viewport[3]);
	glPopAttrib();
}

inline void pop_projection_matrix()
{
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}

void print(const font_data& ft_font, float x, float y, const char* fmt, ...)
{
	pushScreenCoordinateMatrix();
	GLuint font = ft_font.list_base;

	// A little biggeer 
	float h = ft_font.h / 0.63f;
	char text[256];                // String
	va_list ap;

	if (fmt == NULL) {
		*text = 0;         // Do nothing
	}
	else {
		va_start(ap, fmt);          // Parses the string for variables
		vsprintf(text, fmt, ap);    // Converts symbols to actual numbers
		va_end(ap);                 // Results are stored in text
	}
	const char* start_line = text;
	vector<string> lines;

	const char* c;
	for (c = text; *c; ++c) {
		if (*c == '\n') {
			string line;
			for (const char* n = start_line; n < c; n++) {
				line.append(1, *n);
			}
			lines.push_back(line);
			start_line = c + 1;
		}
	}
	if (start_line) {
		string line;
		for (const char* n = start_line; n < c; ++n) {
			line.append(1, *n);
		}
		lines.push_back(line);
	}

	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glListBase(font);

	float modelview_matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);

	for (int i = 0; i < lines.size(); ++i) {
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(x, y - h * i, 0);
		glMultMatrixf(modelview_matrix);
		// If we need to know the length of the text
		// If we decide to use it make sure to also uncomment the glBitmap command in make_dlist()
		// glRasterPos2f(0, 0);
		glCallLists(lines[i].length(), GL_UNSIGNED_BYTE, lines[i].c_str());
		// float rpos[4];
		// glGetFloatv(GL_CURRENT_RASTER_POSITION, rpos);
		// float len = x - rpos[0];
		glPopMatrix();
	}
	glPopAttrib();
	pop_projection_matrix();
}
}

