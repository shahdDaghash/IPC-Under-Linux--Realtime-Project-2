#include <ft2build.h>
#include FT_FREETYPE_H
#include <GL/glut.h>
#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;
 
int WIN_HEIGHT;
int WIN_WIDTH;
int fontSize = 24;
float translationX = -1.0f;
float translationY = -1.0f;

// FreeType library context
FT_Library ftLibrary;

// Font face
FT_Face fontFace;

// Function to initialize FreeType
void initFreeType()
{
    // Initialize FreeType library
    FT_Init_FreeType(&ftLibrary);

    // Load the font face

    // /usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf
    FT_New_Face(ftLibrary, "./resources/UbuntuMono-R.ttf", 0, &fontFace);

    // Set the font size (in pixels)
    FT_Set_Pixel_Sizes(fontFace, 0, fontSize);
}

// Function to render text
void renderText(const char *text, float x, float y, int size)
{
    x = (x + 1) * WIN_WIDTH / 2;
    y = (y + 1) * WIN_HEIGHT / 2;
    float sizeF = (float)size * min(WIN_WIDTH, WIN_HEIGHT) / 600;
    // fontSize = int(sizeF);
    FT_Set_Pixel_Sizes(fontFace, 0, int(sizeF));

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WIN_WIDTH, 0, WIN_HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int textWidth = 0;
    const char *c;
    for (c = text; *c; ++c)
    {
        FT_Load_Char(fontFace, *c, FT_LOAD_RENDER);
        textWidth += (fontFace->glyph->advance.x >> 6);
    }

    // Calculate the starting position to center the text
    float startX = x - (textWidth / 2.0f);

    // Set the position of the text
    glTranslatef(startX, y, 0);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set font rendering parameters
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Iterate over the characters in the text
    for (c = text; *c; ++c)
    {
        // Load the glyph for the current character
        if (FT_Load_Char(fontFace, *c, FT_LOAD_RENDER))
            continue;

        // Access the glyph's bitmap
        FT_Bitmap *bitmap = &(fontFace->glyph->bitmap);

        for (int row = 0; row < bitmap->rows / 2; ++row)
        {
            unsigned char *topRow = bitmap->buffer + row * bitmap->width;
            unsigned char *bottomRow = bitmap->buffer + (bitmap->rows - row - 1) * bitmap->width;
            for (int col = 0; col < bitmap->width; ++col)
            {
                unsigned char temp = topRow[col];
                topRow[col] = bottomRow[col];
                bottomRow[col] = temp;
            }
        }
        fontFace->glyph->bitmap_top = bitmap->rows - fontFace->glyph->bitmap_top;

        // Render the glyph using OpenGL
        glRasterPos2f(fontFace->glyph->bitmap_left, -fontFace->glyph->bitmap_top);
        glDrawPixels(bitmap->width, bitmap->rows, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap->buffer);

        // Move the pen position
        glTranslatef((fontFace->glyph->advance.x >> 6) + 0 * fontSize, 0, 0);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void drawCircle(float r, float x, float y)
{

    float i = 0.0f;
    float aspectRatio_x = 1.0f;
    float aspectRatio_y = 1.0f;
    if (WIN_WIDTH < WIN_HEIGHT)
    {
        aspectRatio_y = (float)WIN_WIDTH / (float)WIN_HEIGHT;
    }
    else
    {
        aspectRatio_x = (float)WIN_HEIGHT / (float)WIN_WIDTH;
    }

    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(x, y); // Center
    for (i = 0.0f; i <= 360; i++)
        glVertex2f(r * cos(M_PI * i / 180.0) * aspectRatio_x + x, r * sin(M_PI * i / 180.0) * aspectRatio_y + y);

    glEnd();
}

// GLUT display function
void display()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0f, 0.0f, 0.0f);

    // Render the text at position (0, 0)
    renderText("Hello, World! yeeeg", 0, 0, 24);

    drawCircle(0.05, translationX ,  translationY);
    glutSwapBuffers();
}

// GLUT reshape function
void reshape(int width, int height)
{

    // // make the view port always square
    // if (width < height)
    // {
    //     glViewport(0, (height - width) / 2, width, width);
    //     WIN_HEIGHT = width;
    //     WIN_WIDTH = width;
    // }
    // else
    // {
    //     glViewport((width - height) / 2, 0, height, height);
    //     WIN_HEIGHT = height;
    //     WIN_WIDTH = height;
    // }
    glViewport(0, 0, width, height);
    WIN_HEIGHT = height;
    WIN_WIDTH = width;
    // float size = 24.0 * min(width, height) / 600;
    // fontSize = int(size);
}

void update(int value)
{
    translationX += 0.01f; // Adjust translation speed as needed
    if(translationX>1.0f){
        return;
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 16 milliseconds between updates (approximately 60 FPS)
}
void update2(int value)
{
    translationY += 0.01f; // Adjust translation speed as needed
    if(translationY>1.0f){
        return;
    }
    
    glutPostRedisplay();
    glutTimerFunc(64, update2, 0); // 16 milliseconds between updates (approximately 60 FPS)
}

// GLUT main function
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("FreeType Text Rendering");

    initFreeType();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, update, 0);
    glutTimerFunc(1000, update2, 0);

    glutMainLoop();

    // Cleanup FreeType resources
    FT_Done_Face(fontFace);
    FT_Done_FreeType(ftLibrary);

    return 0;
}
