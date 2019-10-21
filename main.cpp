//
//  main.cpp
//  consoleFPS
//
//  Created by David Ondrich on 9/6/19.
//  Copyright © 2019 David Ondrich. All rights reserved.
//

#include <iostream>
#include <locale.h>
#include <vector>
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
    // keypad(hconsole, true);
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

        if (user_input == 'w') // Collision Detection Forwards
        {
          fPlayerX += sin(fPlayerA) * 5.0f * fElapsedTime;
          fPlayerY += cos(fPlayerA) * 5.0f * fElapsedTime;

          if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
          {
            fPlayerX -= sin(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cos(fPlayerA) * 5.0f * fElapsedTime;
          }
        }

        if (user_input == 's') // Collision Detection Backwards
        {
          fPlayerX -= sin(fPlayerA) * 5.0f * fElapsedTime;
          fPlayerY -= cos(fPlayerA) * 5.0f * fElapsedTime;

          if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
          {
            fPlayerX += sin(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cos(fPlayerA) * 5.0f * fElapsedTime;
          }
        }

        for(int x=0; x < nScreenWidth; x++)
        {
            // For each column, calculate the projected ray angle into world space
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

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

                        vector<pair<float, float> > p; // distance, dot

                        for (int tx=0; tx < 2; tx++)
                        {
                          for (int ty=0; ty < 2; ty++)
                          {
                            // Angle of corner to eye
                            float vy = (float)nTestY + ty - fPlayerY;
                            float vx = (float)nTestX + tx - fPlayerX;
                            float d = sqrt(vx*vx + vy*vy);
                            float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                            p.push_back(make_pair(d, dot));
                          }
                        }
                        // Sort Pairs from closest to farthest
                        sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });

                        // First two/three are closest (we will never see all four)
                        float fBound = 0.01;
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                        if (acos(p.at(2).second) < fBound) bBoundary = true;
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

            if (bBoundary) nShade = ' ';

            for (int y=0; y<nScreenHeight; y++)
            {
                if (y < nCeiling)
                {
//                    wchar_t nShadeCeiling;
//                    // Shade floor based on distance
//                    float b = 1.0f - (((float)y + nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
//                    if (b < 0.25)		nShadeCeiling  = '#';
//                    else if (b < 0.5)	nShadeCeiling  = 'x';
//                    else if (b < 0.75)	nShadeCeiling  = '.';
//                    else if (b < 0.9)	nShadeCeiling  = '-';
//                    else				nShadeCeiling  = ' ';
//                    screen[y*nScreenWidth + x] = nShadeCeiling;
                    screen[y*nScreenWidth + x] = ' ';
                }
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
        // Display Stats
        wprintf(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f/fElapsedTime);

        // Display Map
        for (int nx = 0; nx < nMapWidth; nx++)
          for (int ny = 0; ny < nMapWidth; ny++)
          {
            screen[(ny+1)*nScreenWidth + nx] = map[ny * nMapWidth + nx];
          }
        screen[((int)fPlayerX+1) * nScreenWidth + (int)fPlayerY] = 'P';
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
