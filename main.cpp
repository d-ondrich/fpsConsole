//
//  main.cpp
//  consoleFPS
//
//  Created by David Ondrich on 9/6/19.
//  Copyright © 2019 David Ondrich. All rights reserved.
//

#include <iostream>
#include <locale.h>
#include <chrono>
#include <math.h>
#include <ncurses.h>
using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

int main() {

    setlocale(LC_ALL, "");
    // Create Screen Buffer
    wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
    initscr();
    WINDOW* hconsole;
    hconsole = newwin(nScreenHeight, nScreenWidth, 0, 0);
//    keypad(hconsole, true);
    noecho();
    curs_set(0);
    wrefresh(hconsole);
    // delwin(hconsole);
    refresh();
    int user_input;
    user_input = getch();
    
    // Map
    wstring map;
    
    map+= L"################";
    map+= L"#..............#";
    map+= L"#..............#";
    map+= L"#..............#";
    map+= L"#..............#";
    map+= L"#..........#...#";
    map+= L"#..........#...#";
    map+= L"#..............#";
    map+= L"#..............#";
    map+= L"#..............#";
    map+= L"#..............#";
    map+= L"#..............#";
    map+= L"#.......########";
    map+= L"#..............#";
    map+= L"#..............#";
    map+= L"#..............#";
    map+= L"################";
    // end map
    
    auto time_point_1 = chrono::system_clock::now();
    auto time_point_2 = chrono::system_clock::now();
    
    // GAME LOOP
    while(1)
    {
        time_point_2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = time_point_2 - time_point_1;
        time_point_1 = time_point_2;
        float fElapsedTime = elapsedTime.count();
        
        
        // Controles
        // Handles rotations
        if (user_input == 'a')
            fPlayerA -= (0.5f) * fElapsedTime;
        
        if (user_input == 'd')
            fPlayerA += (0.5f) * fElapsedTime;

        if (user_input == 'w')
            fPlayerX += sin(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cos(fPlayerA) * 5.0f * fElapsedTime;

        if (user_input == 's')
            fPlayerX -= sin(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cos(fPlayerA) * 5.0f * fElapsedTime;

        for(int x=0; x < nScreenWidth; x++)
        {
            // For each column, calculate the projected ray angle into world space
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
            
            float fDistanceToWall = 0;
            bool bHitWall = false;
            
            float fEyeX = sin(fRayAngle); // unit vector for ray in player space
            float fEyeY = cos(fRayAngle);
            
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += 0.1f;
                
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
                
                // Test if OOB
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    // Ray is inbounds, test to see if the ray cell is a wall block
                    if (map[nTestY * nMapWidth + nTestX] == '#')
                    {
                        bHitWall = true;
                    }
                }
            }
            
            // Calculate distance to ceiling and floor
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            wchar_t nShade = ' ';

            if (fDistanceToWall <= fDepth / 4.0f)               nShade = L'\u2588'; // very close
            else if (fDistanceToWall <= fDepth / 3.0f)          nShade = L'\u2593';
            else if (fDistanceToWall <= fDepth / 2.0f)          nShade = L'\u2592';
            else if (fDistanceToWall <= fDepth)                 nShade = L'\u2591';
            else                                                nShade = ' ';    // Too far away


            for (int y=0; y<nScreenHeight; y++)
            {
                if (y < nCeiling)
                    screen[y*nScreenWidth + x] = ' ';
                else if (y > nCeiling && y <=nFloor)
                    screen[y*nScreenWidth + x] = nShade;
                else
                {
                    wchar_t nShadeFloor;
                    // Shade floor based on distance
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25)		nShadeFloor  = '#';
                    else if (b < 0.5)	nShadeFloor  = 'x';
                    else if (b < 0.75)	nShadeFloor  = '.';
                    else if (b < 0.9)	nShadeFloor  = '-';
                    else				nShadeFloor   = ' ';
                    screen[y*nScreenWidth + x] = nShadeFloor;
//                    screen[y*nScreenWidth + x] = ' ';
                }
            }


        }
        // Display Frame
//        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        wprintf(screen);
        wrefresh(hconsole);
//        refresh();
       user_input = getch();

    }

    endwin();
    return 0;
}
