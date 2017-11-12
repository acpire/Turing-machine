#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <list>
using namespace std;
#pragma comment(lib,"opengl32.lib")

char nextstep = 0;
DWORD id_step_thread = 0;
HANDLE thread_2;
static HINSTANCE hInstance = 0;
int index_prog = 0;
vector<vector<float>> Program;
vector<std::vector<float>> tapes;
std::vector<float> coord;
int steps;
GLuint loadBMP(const char * imagepath , double& width , double& height);
GLuint image;
void add_Tape(HWND hwnd , int& numyape)
{
    RECT rect;
    GetWindowRect(hwnd , &rect);
    double width = rect.right - rect.left;
    double height = rect.bottom - rect.top;
    int sx = 10 , sy = 20;
    float size[8] = { 1,( height / 2 ),1 + sx,( height / 2 ) ,1 + sx,( height / 2 ) + sy ,1 ,( height / 2 ) + sy };
    int i = tapes[numyape].size( ) / 8;
    int up = numyape*sy;
    for ( int z = 0; z < 8; z++ )
    {
        if ( z % 2 == 0 )
        {
            tapes[numyape].push_back(( i*( size[4] + 1 ) + size[z] ));
        }
        else
        {
            tapes[numyape].push_back(size[z] + up + 1);
        }
    }
}
class Instr
{
public:
    int St , StNew;
    vector<char> ch; vector<char> chNew;
    vector<char> TapeMove;
    Instr(size_t size)
    {
        ch.resize(size);
        chNew.resize(size);
        TapeMove.resize(size);
    }
};

class TTyring
{
private:

public:std::vector<Instr> Prog;
       std::vector<list<char>> Tapes;
       int Q = 1;
       int* Point;
       void LoadProg(string FileName);
       void Load(string FileName);
}MT;

void TTyring::LoadProg(string FileName)
{
    ifstream InF(FileName , ios::in);

    if ( !Tapes.empty( ) )
        if ( InF.is_open( ) )
        {
            while ( !InF.eof( ) )
            {
                Instr data(Tapes.size( ));

                InF >> data.St;
                InF >> data.StNew;
                for ( int j = 0; j < Tapes.size( ); j++ )
                    InF >> data.ch[j];

                for ( int j = 0; j < Tapes.size( ); j++ )
                    InF >> data.chNew[j];

                for ( int j = 0; j < Tapes.size( ); j++ )
                    InF >> data.TapeMove[j];
                Prog.push_back(data);
            }
        }

    InF.close( );
}
DWORD WINAPI Work(CONST LPVOID t)
{
    steps = 0;
    HWND hwnd = ( CONST HWND )t;
    while ( MT.Q > 0 )
    {
        int counter = 0;
        int i = 0;
        for ( ; counter != MT.Tapes.size( )/* && Q != Prog[i].St*/ && i < MT.Prog.size( ); i++ )
        {
            counter = 0;
            if ( MT.Q == MT.Prog[i].St )
                for ( int j = 0 , count = 0; j < MT.Tapes.size( ); j++ )
                {
                    auto iter = MT.Tapes[j].begin( );
                    advance(iter , MT.Point[j]);
                    if ( *iter == MT.Prog[i].ch[j] )
                    {
                        index_prog = i;
                        counter++; i--;
                        SendMessage(hwnd , WM_PAINT , 0 , 0);
                    }
                }
        }
        steps++;
        SendMessage(hwnd , 15005 , 0 , 0);
        if ( counter == 0 )
        {
            MessageBox(hwnd , "Ошибка" , "Результат" , MB_OK);
            return 0;
        }
        else
        {
            MT.Q = MT.Prog[i].StNew;
            for ( int j = 0; j < MT.Tapes.size( ); j++ )
            {
                auto iter = MT.Tapes[j].begin( );
                advance(iter , MT.Point[j]);
                *iter = MT.Prog[i].chNew[j];
                if ( *MT.Tapes[j].begin( ) != '#' )
                {
                    MT.Point[j]++;
                    MT.Tapes[j].push_front('#');
                    int c = 0;
                    add_Tape(hwnd , j);
                    SendMessage(hwnd , WM_PAINT , 0 , 0);
                }
                else
                    if ( *--MT.Tapes[j].end( ) != '#' )
                    {
                        MT.Point[j]--;
                        MT.Tapes[j].push_back('#');
                        int c = 0;
                        add_Tape(hwnd , j);
                        SendMessage(hwnd , WM_PAINT , 0 , 0);
                    }

                switch ( MT.Prog[i].TapeMove[j] )
                {
                    case 'R': MT.Point[j]++; SendMessage(hwnd , WM_PAINT , 0 , 0);  break;
                    case 'L': MT.Point[j]--; SendMessage(hwnd , WM_PAINT , 0 , 0);  break;
                    case 'S': break;
                    default: MessageBox(hwnd , "Ошибка" , "Результат" , MB_OK); return 0;
                }
            }

            if ( MT.Q == 0 )
            {
                SendMessage(hwnd , WM_PAINT , 0 , 0);
                MessageBox(hwnd , "Accepted" , "Результат" , MB_OK);
                delete MT.Point;
                return 0;
            }
            else
                if ( MT.Q < 0 )
                {
                    MessageBox(hwnd , "Rejected" , "Результат" , MB_OK);
                    delete MT.Point;
                    return 0;
                }
            if ( nextstep == 1 )
            {
                nextstep = 0;
                SuspendThread(thread_2);
            }
        }
    }
}

void TTyring::Load(string FileName)
{
    ifstream InF(FileName , ios::in);
    string data;
    while ( !InF.eof( ) )
    {
        InF >> data;
        data += '#';
        list<char> tmp_data;
        for ( int i = 0; i < data.size( ); i++ )
        {
            tmp_data.push_back(data[i]);
        }
        tmp_data.push_back('#');
        tmp_data.push_front('#');
        Tapes.push_back(tmp_data);
    }
    Point = new int[Tapes.size( )];
    for ( int i = 0; i < Tapes.size( ); i++ )
    {
        Point[i] = 1;
    }
    InF.close( );
}

void Allcalc(HWND& hWnd , string& data , vector<string>& copytapes , int& index)
{
    RECT rect;
    GetWindowRect(hWnd , &rect);
    double width = rect.right - rect.left;
    double height = rect.bottom - rect.top;
    int sx = 10 , sy = 20;
    float size[8] = { 1,( height / 2 ),1 + sx,( height / 2 ) ,1 + sx,( height / 2 ) + sy ,1 ,( height / 2 ) + sy };
    MT.Load(copytapes[index]);
    MT.LoadProg(data);
    if ( !MT.Tapes.empty( ) && tapes.empty( ) )
    {
        tapes.resize(MT.Tapes.size( ));
        /*for (int i = 0; i < MT.Tapes.size(); i++)
        {
            tapes[i].resize(MT.Tapes[i].size() * 8);
        }*/

        for ( int h = 0 , up = 0; h < tapes.size( ); h++ , up += sy )
        {
            for ( int i = 0; i < MT.Tapes[h].size( ); i++ )
            {
                for ( int z = 0; z < 8; z++ )
                {
                    if ( z % 2 == 0 )
                    {
                        tapes[h].push_back(( i*( size[4] + 1 ) + size[z] ));
                        //	tapes[h][i * 8 + z] = (i*(size[4] + 1) + size[z]);
                    }
                    else
                    {
                        tapes[h].push_back(size[z] + up + 1);/*
                        tapes[h][i * 8 + z] = size[z] + up + 1;*/
                    }
                }
            }
        }
    }
    if ( !MT.Prog.empty( ) )
    {
        Program.resize(MT.Prog.size( ));
        for ( int i = 0; i < MT.Prog.size( ); i++ )
        {
            Program[i].resize(( 2 + MT.Tapes.size( ) * 3 ) * 8);
        }

        for ( int h = 0 , up = -sy; h < Program.size( ); h++ , up -= sy )
        {
            for ( int i = 0; i < Program[h].size( ) / 8; i++ )
            {
                for ( int z = 0; z < 8; z++ )
                {
                    if ( z % 2 == 0 )
                    {
                        Program[h][i * 8 + z] = ( i*( size[4] + 1 ) + size[z] );
                    }
                    else
                    {
                        Program[h][i * 8 + z] = size[z] + up - 1;
                    }
                }
            }
        }
    }
    if ( image == 0 )
    {
        image = loadBMP("1.bmp" , width , height);
        coord.resize(8 * ( 128 ));
        for ( int i = 0 , y = 80 , x = 250 , sx = 35 , sy = 79; i <= coord.size( ) - ( 8 * 96 ); i += 8 )
        {
            coord[i] = x / width;
            coord[i + 1] = ( 1.0f - y ) / height;
            coord[i + 2] = ( x + sx ) / width;
            coord[i + 3] = ( 1.0f - y ) / height;
            coord[i + 4] = ( x + sx ) / width;
            coord[i + 5] = ( 1.0f - y - sy ) / height;
            coord[i + 6] = x / width;
            coord[i + 7] = ( 1.0f - y - sy ) / height;
            y += sy;
        }
        for ( int i = 32 * 8 , y = 80 , x = 975 , sx = 35 , sy = 79; i <= coord.size( ) - ( 8 * 64 ); i += 8 )
        {
            coord[i] = x / width;
            coord[i + 1] = ( 1.0f - y ) / height;
            coord[i + 2] = ( x + sx ) / width;
            coord[i + 3] = ( 1.0f - y ) / height;
            coord[i + 4] = ( x + sx ) / width;
            coord[i + 5] = ( 1.0f - y - sy ) / height;
            coord[i + 6] = x / width;
            coord[i + 7] = ( 1.0f - y - sy ) / height;
            y += sy;
        }

        for ( int i = 64 * 8 , y = 80 , x = 1475 , sx = 35 , sy = 79; i <= coord.size( ) - ( 8 * 32 ); i += 8 )
        {
            coord[i] = x / width;
            coord[i + 1] = ( 1.0f - y ) / height;
            coord[i + 2] = ( x + sx ) / width;
            coord[i + 3] = ( 1.0f - y ) / height;
            coord[i + 4] = ( x + sx ) / width;
            coord[i + 5] = ( 1.0f - y - sy ) / height;
            coord[i + 6] = x / width;
            coord[i + 7] = ( 1.0f - y - sy ) / height;
            y += sy;
        }
        for ( int i = 96 * 8 , y = 80 , x = 1940 , sx = 35 , sy = 79; i < coord.size( ); i += 8 )
        {
            coord[i] = x / width;
            coord[i + 1] = ( 1.0f - y ) / height;
            coord[i + 2] = ( x + sx ) / width;
            coord[i + 3] = ( 1.0f - y ) / height;
            coord[i + 4] = ( x + sx ) / width;
            coord[i + 5] = ( 1.0f - y - sy ) / height;
            coord[i + 6] = x / width;
            coord[i + 7] = ( 1.0f - y - sy ) / height;
            y += sy;
        }
        for ( int i = 0; i < coord.size( ) / 2; i += 8 )
        {
            auto t = coord[i + 7];
            coord[i + 7] = coord[coord.size( ) - i - 1];
            coord[coord.size( ) - i - 1] = t;
            t = coord[i + 6];
            coord[i + 6] = coord[coord.size( ) - i - 2];
            coord[coord.size( ) - i - 2] = t;
            t = coord[i + 5];
            coord[i + 5] = coord[coord.size( ) - i - 3];
            coord[coord.size( ) - i - 3] = t;
            t = coord[i + 4];
            coord[i + 4] = coord[coord.size( ) - i - 4];
            coord[coord.size( ) - i - 4] = t;
            t = coord[i + 3];
            coord[i + 3] = coord[coord.size( ) - i - 5];
            coord[coord.size( ) - i - 5] = t;
            t = coord[i + 2];
            coord[i + 2] = coord[coord.size( ) - i - 6];
            coord[coord.size( ) - i - 6] = t;
            t = coord[i + 1];
            coord[i + 1] = coord[coord.size( ) - i - 7];
            coord[coord.size( ) - i - 7] = t;
            t = coord[i];
            coord[i] = coord[coord.size( ) - i - 8];
            coord[coord.size( ) - i - 8] = t;
        }
        for ( int i = 0 , z = ( coord.size( ) / 4 ) * 3; i + 8 < coord.size( ) / 2; i += 8 , z += 8 )
        {
            auto t = coord[i + 7];
            coord[i + 7] = coord[z + 7];
            coord[z + 7] = t;
            t = coord[i + 6];
            coord[i + 6] = coord[z + 6];
            coord[z + 6] = t;
            t = coord[i + 5];
            coord[i + 5] = coord[z + 5];
            coord[z + 5] = t;
            t = coord[i + 4];
            coord[i + 4] = coord[z + 4];
            coord[z + 4] = t;
            t = coord[i + 3];
            coord[i + 3] = coord[z + 3];
            coord[z + 3] = t;
            t = coord[i + 2];
            coord[i + 2] = coord[z + 2];
            coord[z + 2] = t;
            t = coord[i + 1];
            coord[i + 1] = coord[z + 1];
            coord[z + 1] = t;
            t = coord[i];
            coord[i] = coord[z];
            coord[z] = t;
            if ( ( i + 8 ) % ( coord.size( ) / 4 ) == 0 )
            {
                z = z - ( coord.size( ) / 2 );
            }
        }
    }
}

void display( )
{
    glBindTexture(GL_TEXTURE_2D , image);
    glEnable(GL_TEXTURE_2D);

    glClearColor(1 , 1 , 1 , 1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBegin(GL_QUADS);
    if ( !tapes.empty( ) )
        for ( int z = 0; z < tapes.size( ); z++ )
        {
            if ( !MT.Tapes[z].empty( ) )
            {
                auto i = tapes[z].begin( );
                auto inext = tapes[z].begin( );
                inext++;
                int p = 0;
                for ( auto f = MT.Tapes[z].begin( ); f != MT.Tapes[z].end( ); f++ , i++ , inext++ , p++ )
                {
                    if ( MT.Point[z] == p )
                        glColor3f(0.0f , 1.0f , 0.0f);
                    else
                        glColor3f(1.0f , 1.0f , 1.0f);
                    glTexCoord2d(coord[*f * 8] , coord[*f * 8 + 1]); glVertex3f(*i , *inext , 0);
                    i++; inext++;
                    i++; inext++;
                    glTexCoord2d(coord[*f * 8 + 2] , coord[*f * 8 + 3]); glVertex3f(*i , *inext , 0);
                    i++; inext++;
                    i++; inext++;
                    glTexCoord2d(coord[*f * 8 + 4] , coord[*f * 8 + 5]); glVertex3f(*i , *inext , 0);
                    i++; inext++;
                    i++; inext++;
                    glTexCoord2d(coord[*f * 8 + 6] , coord[*f * 8 + 7]); glVertex3f(*i , *inext , 0);
                    i++; inext++;
                    if ( inext == tapes[z].end( ) )
                        break;
                }
            }
        }
    if ( !Program.empty( ) )
    {
        auto f = MT.Prog.begin( );
        for ( int z = 0 , i = 0; z < Program.size( ) && f != MT.Prog.end( ); f++ , z++ , i = 0 )
        {
            if ( MT.Q == ( f->St ) )
                if ( f->ch[0] == MT.Prog[index_prog].ch[0] )
                    glColor3f(0.8f , 0.6f , 0.4f);
                else
                    glColor3f(1.0f , 1.0f , 0.0f);
            else
                glColor3f(1.0f , 1.0f , 1.0f);

            glTexCoord2d(coord[( f->St + 48 ) * 8] , coord[( f->St + 48 ) * 8 + 1]); glVertex3f(Program[z][i] , Program[z][i + 1] , 0);
            glTexCoord2d(coord[( f->St + 48 ) * 8 + 2] , coord[( f->St + 48 ) * 8 + 3]); glVertex3f(Program[z][i + 2] , Program[z][i + 3] , 0);
            glTexCoord2d(coord[( f->St + 48 ) * 8 + 4] , coord[( f->St + 48 ) * 8 + 5]); glVertex3f(Program[z][i + 4] , Program[z][i + 5] , 0);
            glTexCoord2d(coord[( f->St + 48 ) * 8 + 6] , coord[( f->St + 48 ) * 8 + 7]); glVertex3f(Program[z][i + 6] , Program[z][i + 7] , 0);
            i += 8;
            glTexCoord2d(coord[( f->StNew + 48 ) * 8] , coord[( f->StNew + 48 ) * 8 + 1]); glVertex3f(Program[z][i] , Program[z][i + 1] , 0);
            glTexCoord2d(coord[( f->StNew + 48 ) * 8 + 2] , coord[( f->StNew + 48 ) * 8 + 3]); glVertex3f(Program[z][i + 2] , Program[z][i + 3] , 0);
            glTexCoord2d(coord[( f->StNew + 48 ) * 8 + 4] , coord[( f->StNew + 48 ) * 8 + 5]); glVertex3f(Program[z][i + 4] , Program[z][i + 5] , 0);
            glTexCoord2d(coord[( f->StNew + 48 ) * 8 + 6] , coord[( f->StNew + 48 ) * 8 + 7]); glVertex3f(Program[z][i + 6] , Program[z][i + 7] , 0);
            i += 8;
            for ( int t = 0; t < f->ch.size( ); t++ , i += 8 )
            {
                glTexCoord2d(coord[f->ch[t] * 8] , coord[f->ch[t] * 8 + 1]); glVertex3f(Program[z][i] , Program[z][i + 1] , 0);
                glTexCoord2d(coord[f->ch[t] * 8 + 2] , coord[f->ch[t] * 8 + 3]); glVertex3f(Program[z][i + 2] , Program[z][i + 3] , 0);
                glTexCoord2d(coord[f->ch[t] * 8 + 4] , coord[f->ch[t] * 8 + 5]); glVertex3f(Program[z][i + 4] , Program[z][i + 5] , 0);
                glTexCoord2d(coord[f->ch[t] * 8 + 6] , coord[f->ch[t] * 8 + 7]); glVertex3f(Program[z][i + 6] , Program[z][i + 7] , 0);
            }
            for ( int t = 0; t < f->chNew.size( ); t++ , i += 8 )
            {
                glTexCoord2d(coord[f->chNew[t] * 8] , coord[f->chNew[t] * 8 + 1]); glVertex3f(Program[z][i] , Program[z][i + 1] , 0);
                glTexCoord2d(coord[f->chNew[t] * 8 + 2] , coord[f->chNew[t] * 8 + 3]); glVertex3f(Program[z][i + 2] , Program[z][i + 3] , 0);
                glTexCoord2d(coord[f->chNew[t] * 8 + 4] , coord[f->chNew[t] * 8 + 5]); glVertex3f(Program[z][i + 4] , Program[z][i + 5] , 0);
                glTexCoord2d(coord[f->chNew[t] * 8 + 6] , coord[f->chNew[t] * 8 + 7]); glVertex3f(Program[z][i + 6] , Program[z][i + 7] , 0);
            }
            for ( int t = 0; t < f->TapeMove.size( ); t++ , i += 8 )
            {
                glTexCoord2d(coord[f->TapeMove[t] * 8] , coord[f->TapeMove[t] * 8 + 1]); glVertex3f(Program[z][i] , Program[z][i + 1] , 0);
                glTexCoord2d(coord[f->TapeMove[t] * 8 + 2] , coord[f->TapeMove[t] * 8 + 3]); glVertex3f(Program[z][i + 2] , Program[z][i + 3] , 0);
                glTexCoord2d(coord[f->TapeMove[t] * 8 + 4] , coord[f->TapeMove[t] * 8 + 5]); glVertex3f(Program[z][i + 4] , Program[z][i + 5] , 0);
                glTexCoord2d(coord[f->TapeMove[t] * 8 + 6] , coord[f->TapeMove[t] * 8 + 7]); glVertex3f(Program[z][i + 6] , Program[z][i + 7] , 0);
            }
        }
    }
    glEnd( );

    glDisable(GL_TEXTURE_2D);
    glFlush( );
}

GLuint loadBMP(const char * imagepath , double& width , double& height)
{
    unsigned char header[54];
    unsigned int dataPos;

    unsigned int imageSize;
    unsigned char * data;
    FILE * file = fopen(imagepath , "rb");
    if ( !file )
    {
        printf("Image could not be opened\n");
        return 0;
    }
    if ( fread(header , 1 , 54 , file) != 54 )
    {
        printf("Not a correct BMP file\n");
        return false;
    }
    if ( header[0] != 'B' || header[1] != 'M' )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }
    dataPos = *( int* )&( header[0x0A] );
    imageSize = *( int* )&( header[0x22] );
    width = *( int* )&( header[0x12] );
    height = *( int* )&( header[0x16] );
    if ( imageSize == 0 )
        imageSize = width*height * 3;
    if ( dataPos == 0 )
        dataPos = 54;
    data = new unsigned char[imageSize];
    fread(data , 1 , imageSize , file);
    fclose(file);
    GLuint textureID;
    glGenTextures(1 , &textureID);
    glBindTexture(GL_TEXTURE_2D , textureID);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGB , width , height , 0 , GL_BGR_EXT , GL_UNSIGNED_BYTE , data);

    delete[] data;
    return textureID;
}
HWND      Button_1;
HWND      Button_2;
HWND	  COMBOBox;
HWND      editline;
HWND      viewresult;
string DATA;
vector<string> datatape;
LONG WINAPI
WindowProc(HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam)
{
    static PAINTSTRUCT ps;

    switch ( uMsg )
    {
        case WM_MOUSEWHEEL:
            {
                if ( ( int ) wParam > 0 )
                {
                    float speed = 1.1;
                    glMatrixMode(GL_PROJECTION);
                    float left = 0;
                    GLsizei right = LOWORD(lParam);
                    float bottom = 0;
                    float 	top = HIWORD(lParam); glMatrixMode(GL_MODELVIEW);
                    //glLoadIdentity();
                    glScaled(speed , speed , 0);
                    glMatrixMode(GL_MODELVIEW);
                    InvalidateRect(hWnd , 0 , 1);
                }
                else if ( ( int ) wParam < 0 )
                {
                    float speed = 0.9;
                    glMatrixMode(GL_PROJECTION);
                    float left = 0;
                    GLsizei right = LOWORD(lParam);
                    float bottom = 0;
                    float 	top = HIWORD(lParam); glMatrixMode(GL_MODELVIEW);
                    glScaled(speed , speed , 0);
                    glMatrixMode(GL_MODELVIEW);
                    InvalidateRect(hWnd , 0 , 1);
                }
                break;
            }
        case WM_MOUSEMOVE:
            {
                UINT fwKeys = wParam;
                int xPos = LOWORD(lParam);
                int yPos = HIWORD(lParam);

                if ( fwKeys & MK_LBUTTON )

                {
                    Sleep(5);
                    POINT pt;
                    GetCursorPos(&pt);
                    ScreenToClient(hWnd , &pt);
                    glMatrixMode(GL_MODELVIEW);
                    glTranslatef(pt.x - xPos , yPos - pt.y , 0);
                    InvalidateRect(hWnd , 0 , 1);
                }
                break;
            }
        case WM_CREATE:
            {
                glEnable(GL_COLOR_MATERIAL);
                glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
                vector<string> data;
                data.push_back("Развернутое слово.txt");
                data.push_back("Тандемный повтор.txt");
                data.push_back("Четность.txt");
                data.push_back("Скобки.txt");
                data.push_back("Зеркальное отображение.txt");
                data.push_back("Сложение.txt");
                data.push_back("Умножение.txt");
                data.push_back("Сравнение.txt");
                data.push_back("Троичная система.txt");
                viewresult = CreateWindowEx(0 , "EDIT" , 0 , WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | BS_TEXT | WS_HSCROLL , 640 , 10 , 100 , 40 , hWnd , ( HMENU ) 15005 , hInstance , NULL);
                editline = CreateWindowEx(0 , "EDIT" , 0 , ES_CENTER | WS_CHILD | WS_VISIBLE | BS_TEXT | ES_MULTILINE | WS_VSCROLL /*| ES_AUTOHSCROLL*/ | ES_AUTOVSCROLL , 90 , 45 , 540 , 40 ,
                    hWnd , ( HMENU ) 15004 , hInstance , NULL);
                auto Button_3 = CreateWindowEx(NULL , "button" , "Перезапуск" , WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON , 130 , 10 , 90 , 30 , hWnd , ( HMENU ) 15002 , hInstance , NULL);
                Button_1 = CreateWindowEx(NULL , "button" , "Следующий шаг" , WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON , 10 , 10 , 120 , 30 , hWnd , ( HMENU ) 15001 , hInstance , NULL);
                Button_2 = CreateWindowEx(NULL , "button" , "Запуск" , WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON , 10 , 50 , 70 , 30 , hWnd , ( HMENU ) 15000 , hInstance , NULL);
                COMBOBox = CreateWindow(( "COMBOBOX" ) , NULL , WS_CHILD | WS_VISIBLE | LBS_STANDARD , 230 , 10 , 400 , 180 , hWnd , ( HMENU ) 15003 , hInstance , NULL);
                for ( auto ptr = data.begin( ); ptr != data.end( ); ptr++ )
                {
                    SendMessage(COMBOBox , CB_ADDSTRING , 0 , ( LPARAM ) ptr->data( ));
                }
                break;
            }
        case WM_PAINT:

            BeginPaint(hWnd , &ps);

            display( );
            EndPaint(hWnd , &ps);
            return 0;

        case WM_SIZE:
            {
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity( );
                float left = 0;
                GLsizei right = LOWORD(lParam);
                float bottom = 0;
                float 	top = HIWORD(lParam);
                glOrtho(left , right , bottom , top , 0 , 1);
                glViewport(0 , 0 , right , top);
                glMatrixMode(GL_MODELVIEW);

                //glLoadIdentity();
                //if (!MT.Tapes.empty())
                //	glTranslated(/*-10 * MT.Tapes.at(0).size() / 2*/0, 0, 0);
                //InvalidateRect(hWnd, 0, 0);
                break;
            }

        case WM_KEYDOWN:
            {

            switch ( wParam )
            {
                case 27:
                    PostQuitMessage(0);
                    break;
                case VK_UP:
                    {
                        glMatrixMode(GL_PROJECTION);
                        glTranslatef(0 , 20 , 0);
                        InvalidateRect(hWnd , 0 , 1);
                        break;
                    }

                case VK_DOWN:
                    {
                        glMatrixMode(GL_PROJECTION);
                        glTranslatef(0 , -20 , 0);
                        InvalidateRect(hWnd , 0 , 1);
                        break;
                    }
                case VK_LEFT:
                    glMatrixMode(GL_PROJECTION);
                    glTranslatef(-20 , 0 , 0);
                    InvalidateRect(hWnd , 0 , 1);
                    break;
                case VK_RIGHT:
                    glMatrixMode(GL_PROJECTION);
                    glTranslatef(20 , 0 , 0);
                    InvalidateRect(hWnd , 0 , 1);
                    break;
            }
            }
            return 0;
        case 15005:
            {
                string data;
                ltoa(steps , const_cast< char* >( data.data( ) ) , 10);
                SetWindowText(viewresult , data.c_str( ));
                //InvalidateRect(hWnd, 0, 0);
                break;
            }
        case WM_CLOSE:
            DestroyWindow(hWnd);
            DestroyWindow(Button_1);
            PostQuitMessage(0);
            break;
        case WM_DESTROY:
            KillTimer(hWnd , 1);
            PostQuitMessage(0);
            break;
        case WM_COMMAND:
            switch ( LOWORD(wParam) )
            {
                case 15000:
                    {
                        if ( !DATA.empty( ) )
                        {
                            if ( id_step_thread == 0 )
                                thread_2 = CreateThread(0 , 0 , Work , hWnd , 0 , &id_step_thread);

                            nextstep = 0;
                        }
                        SetActiveWindow(hWnd);
                        break;
                    }

                case 15001:
                    if ( !DATA.empty( ) )
                    {
                        if ( id_step_thread == 0 )
                        {
                            thread_2 = CreateThread(0 , 0 , Work , hWnd , 0 , &id_step_thread);

                            nextstep = 1;
                        }
                        else if ( nextstep == 0 )
                        {
                            nextstep = 1;
                            ResumeThread(thread_2);
                        }
                    }
                    SetActiveWindow(hWnd);
                    break;
                case  15002:
                    {
                        if ( !DATA.empty( ) )
                        {
                            if ( id_step_thread != 0 )
                            {
                                PostThreadMessage(id_step_thread , WM_QUIT , 0 , 0);
                                //WaitForSingleObject(thread_2, INFINITE);
                                CloseHandle(thread_2);
                                id_step_thread = 0;
                                thread_2 = 0;
                            }
                            MT.Q = 1;
                            MT.Prog.clear( );
                            MT.Tapes.clear( );
                            int i = 0;
                            Allcalc(hWnd , DATA , datatape , i);
                            InvalidateRect(hWnd , 0 , 0);
                        }
                        SetActiveWindow(hWnd);
                        break;
                    }
                case 15003:
                    {
                        if ( HIWORD(wParam) == CBN_SELENDOK )
                        {
							//TerminateThread(thread_2, 0);
                            if ( id_step_thread != 0 )
                            {
                                PostThreadMessage(id_step_thread , WM_QUIT , 0 , 0);
                               // WaitForSingleObject(thread_2, INFINITE);
                                CloseHandle(thread_2);
                                id_step_thread = 0;
                                thread_2 = 0;
                            }

                            index_prog = 0;
                            MT.Q = 1;
                            MT.Prog.clear( );
                            int size = 0;
                            MT.Tapes.clear( );
                            DATA.resize(64);
                            int i = SendMessage(COMBOBox , CB_GETCURSEL , 0 , 0);
                            SendMessage(COMBOBox , CB_GETLBTEXT , i , ( LPARAM ) DATA.data( ));

                            DATA.resize(1 + strlen(DATA.data( )));
                            vector<string> file_tapes;
                            file_tapes.push_back("РС.txt");
                            file_tapes.push_back("ТП.txt");
                            file_tapes.push_back("Ч.txt");
                            file_tapes.push_back("С.txt");
                            file_tapes.push_back("ЗО.txt");
                            file_tapes.push_back("Слож.txt");
                            file_tapes.push_back("УМНОЖ.txt");
                            file_tapes.push_back("СРАВН.txt");
                            file_tapes.push_back("ТРОИЧНАЯ.txt");
                            datatape.clear( );
                            datatape.push_back(file_tapes[i]);
                            Allcalc(hWnd , DATA , file_tapes , i);

                            if ( MT.Tapes.size( ) == 1 )
                            {
                                string data;
                                for ( auto i = MT.Tapes[0].begin( ); i != MT.Tapes[0].end( ); i++ )
                                {
                                    data.push_back(*i);
                                }
                                SetWindowText(editline , data.c_str( ));
                            }

                            for ( int t = 0; t < MT.Tapes.size( ); t++ )
                            {
                                size = MT.Tapes[t].size( ) - tapes[t].size( ) / 8;
                                for ( int i = 0; i < size; i++ )
                                {
                                    add_Tape(hWnd , t);
                                }
                            }
                            InvalidateRect(hWnd , 0 , 0);
                        }
                        SetActiveWindow(hWnd);
                        break;
                    }
                case 15004:
                    {
                        if ( HIWORD(wParam) == EN_CHANGE )
                        {
                            if ( MT.Tapes.size( ) == 1 )
                            {
                                if ( id_step_thread != 0 )
                                {
                                    PostThreadMessage(id_step_thread , WM_QUIT , 0 , 0);
                                    //WaitForSingleObject(thread_2, INFINITE);
                                    CloseHandle(thread_2);
                                    id_step_thread = 0;
                                    thread_2 = 0;
                                }
                                MT.Q = 1;

                                index_prog = 0;
                                MT.Tapes.clear( );

                                int sz = GetWindowTextLength(editline);
                                char* data = new char[sz];
                                GetWindowText(editline , data , sz);

                                list<char> tape;
                                for ( auto i = 0; i < sz - 1; i++ )
                                    tape.push_back(data[i]);
                                tape.push_back('#');
                                delete[] data;
                                MT.Tapes.push_back(tape);
                                add_Tape(hWnd , index_prog);

                                InvalidateRect(hWnd , 0 , 0);
                            }

                            SetActiveWindow(hWnd);
                        }
                        break;
                    }
            }

            break;
    }

    return DefWindowProc(hWnd , uMsg , wParam , lParam);
}

HWND
CreateOpenGLWindow(char* title , int x , int y , int width , int height ,
    BYTE type , DWORD flags)
{
    int         pf;
    HDC         hDC;
    HWND        hWnd;
    WNDCLASS    wc;
    PIXELFORMATDESCRIPTOR pfd;

    if ( !hInstance )
    {
        hInstance = GetModuleHandle(NULL);
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = ( WNDPROC ) WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(NULL , IDI_WINLOGO);
        wc.hCursor = LoadCursor(NULL , IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = "OpenGL";

        if ( !RegisterClass(&wc) )
        {
            MessageBox(NULL , "RegisterClass() failed:  "
                "Cannot register window class." , "Error" , MB_OK);
            return NULL;
        }
    }

    hWnd = CreateWindowEx(WS_EX_ACCEPTFILES , "OpenGL" , title , WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN
        ,
        x , y , width , height , NULL , NULL , hInstance , NULL);

    if ( hWnd == NULL )
    {
        MessageBox(NULL , "CreateWindow() failed:  Cannot create a window." ,
            "Error" , MB_OK);
        return NULL;
    }

    hDC = GetDC(hWnd);
    memset(&pfd , 0 , sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
    pfd.iPixelType = type;
    pfd.cColorBits = 32;

    pf = ChoosePixelFormat(hDC , &pfd);

    SetPixelFormat(hDC , pf , &pfd);

    DescribePixelFormat(hDC , pf , sizeof(PIXELFORMATDESCRIPTOR) , &pfd);

    ReleaseDC(hWnd , hDC);

    return hWnd;
}

int APIENTRY
WinMain(HINSTANCE hCurrentInst , HINSTANCE hPreviousInst ,
    LPSTR lpszCmdLine , int nCmdShow)
{
    HDC hDC;
    HGLRC hRC;
    HWND  hWnd;
    MSG   msg;

    hWnd = CreateOpenGLWindow("Turing" , 0 , 0 , 1280 , 720 , PFD_TYPE_RGBA , 0);
    if ( hWnd == NULL )
        exit(1);

    hDC = GetDC(hWnd);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC , hRC);

    ShowWindow(hWnd , nCmdShow);

    HACCEL hAccelTable = LoadAccelerators(hInstance , MAKEINTRESOURCE(110));
    while ( GetMessage(&msg , nullptr , 0 , 0) )
    {
        if ( !TranslateAccelerator(msg.hwnd , hAccelTable , &msg) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    wglMakeCurrent(NULL , NULL);
    ReleaseDC(hWnd , hDC);
    wglDeleteContext(hRC);
    DestroyWindow(hWnd);

    return msg.wParam;
}