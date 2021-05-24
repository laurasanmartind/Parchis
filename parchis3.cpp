// Laura San Martín
// Optional part not included
// Compiled in Linux

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#undef max
#endif

using namespace std;

const int NUM_PLAYERS = 4;
const int NUM_MARKERS = 4;
const int NUM_SPACES = 68;

typedef enum
{
    Yellow,
    Blue,
    Red,
    Green,
    Gray,
    None
} tColor;

typedef int tMarkers[NUM_MARKERS];

struct tPlayer
{
    tColor color;
    tMarkers markers;
};

typedef tPlayer tPlayers[NUM_PLAYERS];

struct tSpace
{
    tColor lane1;
    tColor lane2;
};

typedef tSpace tSpaces[NUM_SPACES];

struct tGame
{
    tSpaces spaces;
    tPlayers players;
    tColor playerTurn;
    int roll = 0;
    int award = 0;
    int sixes = 0;
    int lastMarkerMoved = 0;
    bool awardWon = false;
};

const string FileName = "0.txt";
const bool Debug = true;

int startSpace(tColor player);
int zanataSpace(tColor player);
int howMany(const tMarkers player, int space);
int firstAt(const tMarkers player, int space);
int dice();
int secondAt(const tMarkers player, int space);

void display(const tGame &game);
void setColor(tColor color);
void pause(bool &end);
void universalPause();
void initColors();
void markerOut(tGame &game);
void toHome(tGame &game, int space);
void initialize(tGame &game);
void load(tGame &game, ifstream &file);
void move(tGame &game, int marker, int space);
void openBridge(tGame &game, int space, int space2);

bool canMove(tGame &game, int marker, int &space);
bool process6(tGame &game, bool &nextPlayer);
bool allAtGoal(const tMarkers player);
bool play(tGame &game, bool &end);
bool isSafe(int space);
bool process5(tGame &game, bool &nextPlayer);

string colorToStr(tColor color);

int main()
{
    tGame game;

    int numOfMarkers = 0;

    bool end = false;
    bool nextPlayer = true;
    bool pFive = false;
    bool pSix = false;
    bool canPlay = false;

    ifstream file;

    //The game starts

    initColors();
    initialize(game);

    if (Debug)
    {
        load(game, file);
    }

    display(game);

    //Main loop
    while (!end)
    {
        setColor(game.playerTurn);
        cout << "Turn for the " << colorToStr(game.playerTurn) << " player" << endl;

        if (game.award > 0)
        {
            cout << "The " << colorToStr(game.playerTurn) << " player has " << game.award << " extra moves!" << endl;
            game.awardWon = false;
            game.roll = game.award;
            canPlay = play(game, end);
            pause(end);

            if (!game.awardWon)
            {
                game.award = 0;
                if (game.sixes == 0)
                {
                    nextPlayer = true;
                }
            }

            else if (canPlay)
            {
                game.award = 0;
                nextPlayer = true;
            }
            else
            {
                nextPlayer = false;
            }
        }
        else
        {
            if (Debug)
            {
                if (file.is_open())
                {
                    file >> game.roll;

                    if (game.roll == -1)
                    {
                        file.close();
                        cout << "Roll (0 to exit):";
                        cin >> game.roll;
                        cin.get();
                    }
                    cout << "Roll = " << game.roll << endl;
                }
                else if (!file.is_open())
                {
                    cout << "Roll (0 to exit):";
                    cin >> game.roll;
                    cin.get();
                }
            }
            else
            {
                game.roll = rand() % 6 + 1;
                cout << "Roll = " << game.roll << endl;
                pause(end);
            }

            numOfMarkers = howMany(game.players[game.playerTurn].markers, -1);
            if ((numOfMarkers == 4) && (game.roll != 5))
            {
                if ((game.roll == 6) && (game.sixes < 3))
                {
                    nextPlayer = false;
                }
            }
            else if (game.roll == 5)
            {
                pFive = process5(game, nextPlayer);
                pSix = false;
                game.sixes = 0;

                if (pFive)
                {
                    markerOut(game);
                    cout << "Marker leaves home" << endl;
                    pause(end);
                    if (game.award == 0)
                    {
                        nextPlayer = true;
                    }
                }
            }

            if (game.roll == 6)
            {
                pSix = process6(game, nextPlayer); // forces bridges openings
                pFive = false;
            }

            if (((!pFive) && (!pSix)) || (game.roll < 5))
            {
                if (game.roll < 6)
                {
                    game.sixes = 0;
                }
                canPlay = play(game, end);
                if (!canPlay)
                {
                    nextPlayer = false;
                }
                if ((game.sixes == 0) && (game.award == 0))
                {
                    nextPlayer = true;
                }
            }
        }

        if (game.roll != 0)

        {
            display(game);

            if (allAtGoal(game.players[game.playerTurn].markers))
            {
                setColor(game.playerTurn);
                cout << "The winner of the game is the " << colorToStr(game.playerTurn) << " player :) !" << endl;
                end = true;
            }
            else if (nextPlayer)
            {
                game.sixes = 0;
                game.playerTurn = tColor((int(game.playerTurn) + 1) % NUM_PLAYERS);
            }
        }
        else
        {
            end = true;
        }
    }
    return 0;
}

void load(tGame &game, ifstream &file) // For debug
{

    int player, space, roll;

    file.open(FileName);
    if (file.is_open())
    {
        for (int i = 0; i < NUM_PLAYERS; i++)
            for (int f = 0; f < NUM_MARKERS; f++)
            {
                file >> space;
                game.players[i].markers[f] = space;
                if ((space >= 0) && (space < NUM_SPACES))
                    if (game.spaces[space].lane1 == None)
                        game.spaces[space].lane1 = tColor(i);
                    else
                        game.spaces[space].lane2 = tColor(i);
            }
        file >> player;
        game.playerTurn = tColor(player);
    }
    else
    {
        cout << "File couldn't be found!" << endl;
        universalPause();
    }
}

int dice() // Returns a random number between 1 and 6
{
    int value;
    srand(time(0));
    value = 1 + rand() % 6;

    cout << "Rolled: " << value << endl;
    return value;
}

bool isSafe(int space) //Spaces 0, 5, 12, 17, 22, 29, 34, 39, 46, 51, 56, 63 and the starts are safe spaces
{
    bool ok = false;

    if ((space % 17 == 0) || (space % 17 == 5) || (space % 17 == 12))
    {
        ok = true;
    }

    return ok;
}

int startSpace(tColor player) // The start for the yellow marker is space 5, the one for the blue marker is 22, the one for the red marker is 39 and the one for the green marker is 56
{
    int position = -1;
    if (player == Yellow)
    {
        position = 5;
    }
    else if (player == Blue)
    {
        position = 22;
    }
    else if (player == Red)
    {
        position = 39;
    }
    else if (player == Green)
    {
        position = 56;
    }
    return position;
}

int zanataSpace(tColor player) // The zanata (the gate to the goal) for each marker is the space that is 5 numbers before its start space
{
    int zanata;
    zanata = (startSpace(player) - 5);

    return zanata;
}

string colorToStr(tColor color) //Returns the string for that colour
{
    string str;
    if (color == Red)
        str = "red";
    if (color == Green)
        str = "green";
    if (color == Yellow)
        str = "yellow";
    if (color == Blue)
        str = "blue";

    return str;
}

void initialize(tGame &game) // Initializes: The random number generator, all markers at home, the colors, and a random player to start
{
    tColor color;
    srand(time(NULL));

    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        for (int j = 0; j < NUM_MARKERS; j++)
        {
            game.players[i].markers[j] = -1;
        }
    }

    for (int i = 0; i < NUM_SPACES; i++)
    {
        game.spaces[i].lane1 = None;
        game.spaces[i].lane2 = None;
    }

    int value;
    srand(time(0));
    value = rand() % 4;

    color = tColor(value);
    game.playerTurn = color;

    setColor(Gray);
}

int howMany(const tMarkers player, int space) //Returns the number of markers of the player in that space
{
    int numOfMarkers = 0;
    for (int i = 0; i < NUM_MARKERS; i++)
    {
        if (space == player[i])
        {
            numOfMarkers++;
        }
    }
    return numOfMarkers;
}

int firstAt(const tMarkers player, int space) // Lowest index of the player’s markers that are in that space
{
    int i = 0;
    int num = -1;
    bool done = false;

    while ((i < NUM_MARKERS) && (!done))
    {
        if (space == player[i])
        {
            num = i;
            done = true;
        }
        i++;
    }
    return num;
}

int secondAt(const tMarkers player, int space) // Highest index of the player’s markers that are in that space
{
    int i = NUM_MARKERS - 1;
    int num = -1;
    bool done = false;

    while ((i >= 0) && (!done))
    {
        if (space == player[i])
        {
            num = i;
            done = true;
        }
        i--;
    }
    return num;
}

void display(const tGame &game) // Board
{
    int space, marker;
    tColor jug;

    cout << "\x1b[2J\x1b[H"; // Go to the upper left square
    setColor(Gray);
    cout << endl;

    // Rows with space numbers...
    for (int i = 0; i < NUM_SPACES; i++)
        cout << i / 10;
    cout << endl;
    for (int i = 0; i < NUM_SPACES; i++)
        cout << i % 10;
    cout << endl;

    // Upper border...
    for (int i = 0; i < NUM_SPACES; i++)
        cout << '>';
    cout << endl;

    // Second lane of spaces...
    for (int i = 0; i < NUM_SPACES; i++)
    {
        setColor(game.spaces[i].lane2);
        if (game.spaces[i].lane2 != None)
            cout << secondAt(game.players[game.spaces[i].lane2].markers, i) + 1;
        else
            cout << ' ';
        setColor(Gray);
    }
    cout << endl;

    // "Median"
    for (int i = 0; i < NUM_SPACES; i++)
        if (isSafe(i))
            cout << 'o';
        else
            cout << '-';
    cout << endl;

    // First lane of spaces...
    for (int i = 0; i < NUM_SPACES; i++)
    {
        setColor(game.spaces[i].lane1);
        if (game.spaces[i].lane1 != None)
            cout << firstAt(game.players[game.spaces[i].lane1].markers, i) + 1;
        else
            cout << ' ';
        setColor(Gray);
    }
    cout << endl;

    jug = Yellow;
    // Lower border...
    for (int i = 0; i < NUM_SPACES; i++)
        if (i == zanataSpace(jug))
        {
            setColor(jug);
            cout << "V";
            setColor(Gray);
        }
        else if (i == startSpace(jug))
        {
            setColor(jug);
            cout << "^";
            setColor(Gray);
            jug = tColor(int(jug) + 1);
        }
        else
            cout << '>';
    cout << endl;

    // Goal paths and homes...
    for (int i = 0; i < NUM_MARKERS; i++)
    {
        space = 0;
        jug = Yellow;
        setColor(jug);
        while (space < NUM_SPACES)
        {
            if (space == zanataSpace(jug))
            {
                marker = firstAt(game.players[jug].markers, 101 + i);
                if (marker != -1)
                {
                    cout << marker + 1;
                    if (howMany(game.players[jug].markers, 101 + i) > 1)
                    {
                        marker = secondAt(game.players[jug].markers, 101 + i);
                        if (marker != -1)
                        {
                            cout << marker + 1;
                        }
                        else
                            cout << "V";
                    }
                    else
                        cout << "V";
                }
                else
                    cout << "VV";
                space++;
            }
            else if (space == startSpace(jug))
            {
                if (game.players[jug].markers[i] == -1) // At home
                    cout << i + 1;
                else
                    cout << "^";
                jug = tColor(int(jug) + 1);
                setColor(jug);
            }
            else
                cout << ' ';
            space++;
        }
        cout << endl;
    }

    // More goal paths...
    for (int i = 105; i <= 107; i++)
    {
        space = 0;
        jug = Yellow;
        setColor(jug);
        while (space < NUM_SPACES)
        {
            if (space == zanataSpace(jug))
            {
                marker = firstAt(game.players[jug].markers, i);
                if (marker != -1)
                {
                    cout << marker + 1;
                    if (howMany(game.players[jug].markers, i) > 1)
                    {
                        marker = secondAt(game.players[jug].markers, i);
                        if (marker != -1)
                        {
                            cout << marker + 1;
                        }
                        else
                            cout << "V";
                    }
                    else
                        cout << "V";
                }
                else
                    cout << "VV";
                space++;
                jug = tColor(int(jug) + 1);
                setColor(jug);
            }
            else
                cout << ' ';
            space++;
        }
        cout << endl;
    }

    space = 0;
    jug = Yellow;
    setColor(jug);
    while (space < NUM_SPACES)
    {
        cout << ((game.players[jug].markers[0] == 108) ? '1' : '.');
        cout << ((game.players[jug].markers[1] == 108) ? '2' : '.');
        jug = tColor(int(jug) + 1);
        setColor(jug);
        cout << "               ";
        space += 17;
    }
    cout << endl;
    space = 0;
    jug = Yellow;
    setColor(jug);
    while (space < NUM_SPACES)
    {
        cout << ((game.players[jug].markers[2] == 108) ? '3' : '.');
        cout << ((game.players[jug].markers[3] == 108) ? '4' : '.');
        jug = tColor(int(jug) + 1);
        setColor(jug);
        cout << "               ";
        space += 17;
    }
    cout << endl
         << endl;
    setColor(Gray);
}

void setColor(tColor color)
{
    switch (color)
    {
    case Yellow:
        cout << "\x1b[33;107m";
        break;
    case Blue:
        cout << "\x1b[34;107m";
        break;
    case Red:
        cout << "\x1b[31;107m";
        break;
    case Green:
        cout << "\x1b[32;107m";
        break;
    case Gray:
    case None:
        cout << "\x1b[90;107m";
        break;
    }
}

/*void pause()
{
   cout << "Press Enter to continue...";
   cin.ignore(numeric_limits<streamsize>::max(), '\n');
}*/

void universalPause() //Universal replacement of system("pause")
{
    while (getchar() != '\n')
        ;
}

void pause(bool &end)
{
    char key = '1';

    cout << "Press Enter to continue . . .";
    while (key != '\n')
    {
        key = getchar();

        if (key == '0')
        {
            end = true;
            cout << "Exiting" << endl;
        }
    }
}

void initColors()
{
#ifdef _WIN32
    for (DWORD stream : {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE})
    {
        DWORD mode;
        HANDLE handle = GetStdHandle(stream);

        if (GetConsoleMode(handle, &mode))
        {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(handle, mode);
        }
    }
#endif
}

void markerOut(tGame &game) //Gets the marker from the playerTurn out of home
{
    int space;
    int first;

    space = startSpace(game.playerTurn);

    first = firstAt(game.players[game.playerTurn].markers, -1);
    game.players[game.playerTurn].markers[first] = space;

    if (game.spaces[space].lane1 == None)
    {
        game.spaces[space].lane1 = game.playerTurn;
    }
    else
    {
        game.spaces[space].lane2 = game.playerTurn;
    }
}

void toHome(tGame &game, int space) //Sends home the marker that is in that space on lane2
{
    tColor color;
    int marker;

    color = game.spaces[space].lane2;
    marker = secondAt(game.players[color].markers, space);
    game.players[color].markers[marker] = -1;
    game.spaces[space].lane2 = None;
}

bool process5(tGame &game, bool &nextPlayer) //Tries to get out of home a marker from the playerTurn
{
    bool ok = false;
    int space;

    space = startSpace(game.playerTurn);
    if (firstAt(game.players[game.playerTurn].markers, -1) != -1)
    {
        if (game.spaces[space].lane2 == None) //If there is no marker in the start on lane2, a marker from the playerTurn can get out of home
        {
            ok = true;
        }
        else //If there is a marker
        {
            if (game.playerTurn != game.spaces[space].lane2) //and it is not the player's the one on lane2 is sent home, a marker gets out of home and 20 will be counted as a reward, not having to pass the turn, because he must be able to play the reward next
            {
                toHome(game, space);
                ok = true;
                game.award = 20;
                nextPlayer = false;
            }
            else // If it is, there are two of his markers and he can't get another one
            {
                ok = false;
            }
        }
    }
    return ok;
}

bool allAtGoal(const tMarkers player) // Returns true if all the markers of the player are at the goal, and false in other case
{
    bool ok = false;

    if (howMany(player, 108) >= NUM_MARKERS)
    {
        ok = true;
    }

    return ok;
}
void openBridge(tGame &game, int space, int space2) // Opens the bridge the playerTurn has in that space
{
    int marker;

    for (int i = 0; i < NUM_MARKERS; i++)
    {
        if (game.players[game.playerTurn].markers[i] == space)
            marker = i;
    }
    cout << "The marker number " << marker + 1 << " of the " << colorToStr(game.playerTurn) << " player, has to move to space: " << space2 << endl;
    universalPause();
    move(game, marker, space2);

    game.lastMarkerMoved = marker;
}

bool process6(tGame &game, bool &nextPlayer) // A six may force some movements. Opening a bridge or sending home the last marker moved
{
    bool ok = true;
    int bridge1Space = -2;
    int bridge1Marker1 = -2;
    int bridge1Marker2 = -2;
    int bridge2Space = -2;
    int bridge2Marker1 = -2;
    int bridge2Marker2 = -2;
    int bridgeChosen = -2;
    int initSpace1 = -2;
    int initSpace2 = -2;

    game.sixes++;

    if ((howMany(game.players[game.playerTurn].markers, -1)) == 0) //If the player has no markers at home, the roll will become a 7
    {
        game.roll = 7;
        cout << "Roll = 6. All markers are out of home. Roll = 7 " << endl;
    }

    if (game.sixes > 2)
    {
        if ((game.players[game.playerTurn].markers[game.lastMarkerMoved] <= 100) && (howMany(game.players[game.playerTurn].markers, -1) != 4)) //If it is not on the path to the goal, it is sent home, placing it first on lane2
        {
            cout << "Oh no!! Third consecutive 6!! To home" << endl;
            universalPause();
            if (game.spaces[game.players[game.playerTurn].markers[game.lastMarkerMoved]].lane1 == game.spaces[game.players[game.playerTurn].markers[game.lastMarkerMoved]].lane2)
            {
                game.spaces[game.players[game.playerTurn].markers[game.lastMarkerMoved]].lane2 == None;
            }
            else
            {
                game.spaces[game.players[game.playerTurn].markers[game.lastMarkerMoved]].lane1 = None;
                game.spaces[game.players[game.playerTurn].markers[game.lastMarkerMoved]].lane2 = game.playerTurn;
            }
            toHome(game, game.players[game.playerTurn].markers[game.lastMarkerMoved]);
        }
        else if (howMany(game.players[game.playerTurn].markers, -1) == 4)
        {
            cout << "Third consecutive 6!!" << endl;
            universalPause();
        }
        else
        {
            cout << "Third six in a row...The last marker moved is beyond the zanata and it's not sent home!" << endl;
            universalPause();
        }
        nextPlayer = true;
    }
    else
    {
        nextPlayer = false;
        for (int i = 0; i < NUM_MARKERS; i++) // It will be necessary to check if there is some bridge to open
        {
            for (int j = i + 1; j < NUM_MARKERS; j++)
            {
                if ((game.players[game.playerTurn].markers[i] == game.players[game.playerTurn].markers[j]) && (game.players[game.playerTurn].markers[i] != -1))
                {
                    if (bridge1Space == -2)
                    {
                        bridge1Space = game.players[game.playerTurn].markers[i];
                        bridge1Marker1 = i;
                        bridge1Marker2 = j;
                        initSpace1 = bridge1Space;
                    }
                    else if (bridge2Space == -2)
                    {
                        bridge2Space = game.players[game.playerTurn].markers[i];
                        bridge2Marker1 = i;
                        bridge2Marker2 = j;
                        initSpace2 = bridge2Space;
                    }
                }
            }
        }
        //If there is one bridge and the markers can be moved roll spaces, that bridge is opened and the function process6()returns true
        if ((bridge1Space != -2) && (bridge2Space == -2))
        {
            if (canMove(game, bridge1Marker1, bridge1Space))
            {
                cout << "Rolled : 6. Opening bridge" << endl;
                universalPause();
                openBridge(game, initSpace1, bridge1Space);
            }
            else
            {
                ok = false;
            }
        }
        else if (bridge2Space != -2) // If there are two bridges
        {

            if ((canMove(game, bridge1Marker1, bridge1Space)) &&
                (canMove(game, bridge2Marker1, bridge2Space))) // If the markers of both bridges can be moved, the user will decide
            {

                cout << "Choose which bridge to open: "
                     << "[1: From " << initSpace1 << " to space " << bridge1Space << ", 2: From " << initSpace2 << " to space " << bridge2Space << "]:";
                cin >> bridgeChosen;

                if (bridgeChosen == 1)
                {
                    openBridge(game, initSpace1, bridge1Space);
                }
                else if (bridgeChosen == 2)
                {
                    openBridge(game, initSpace2, bridge2Space);
                }
            }
            else if (canMove(game, bridge1Marker1, bridge1Space)) // If only the markers of one bridge can be moved, that bridge is opened
            {
                cout << "Only the bridge on space " << bridge1Space << " can be opened" << endl;
                openBridge(game, initSpace1, bridge1Space);
            }
            else if (canMove(game, bridge2Marker1, bridge2Space))
            {
                cout << "Only the bridge on space " << bridge2Space << " can be opened" << endl;
                openBridge(game, initSpace2, bridge2Space);
            }
            else // If no marker is moved, the function process6()returns false
            {
                cout << "2 bridges. Can't move" << endl;
                ok = false;
                //nextPlayer = true;
            }
        }
        else
        {
            ok = false;
            nextPlayer = false;
        }
    }

    return ok;
}

bool play(tGame &game, bool &end) //Moves , if possible, a marker of the playerTurn roll spaces. It will return true if the turn must go to the next player
{
    bool ok = true;
    bool cm;
    int space;
    int markerToMove;
    bool playerMarkers[NUM_MARKERS];
    int markerCan = 0;

    for (int marker = 0; marker < NUM_MARKERS; marker++) // It will be found out, with canMove(), how many markers can be moved
    {
        space = game.players[game.playerTurn].markers[marker];
        cm = canMove(game, marker, space);
        if (cm)
        {
            cout << marker + 1 << ": "
                 << "can move to position:" << space << endl;
            playerMarkers[marker] = true;
            markerCan++;
        }
        else
        {
            cout << marker + 1 << ": "
                 << "can't move" << endl;
            playerMarkers[marker] = false;
        }
    }
    if (markerCan == 1) // If only one can be moved, it will be moved and the user informed
    {
        for (int marker = 0; marker < NUM_MARKERS; marker++)
        {
            if (playerMarkers[marker] == true)
                markerToMove = marker;
        }
        cout << "Only one marker can be moved!" << endl;
    }
    else if (markerCan == 0) // If no markers can be moved, the user will be informed
    {
        cout << "Next player..." << endl;
        universalPause();
    }
    else // If several markers can be moved, the user will be presented a menu with the moveable markers for him/her to choose
    {
        cout << "Marker to move: ";
        cin >> markerToMove;
        if (markerToMove == 0)
        {
            end = true;
        }
        else
        {
            while (((markerToMove > 0) && (playerMarkers[markerToMove - 1] == false)) || (markerToMove > 4))
            {
                if ((markerToMove > 0) && (playerMarkers[markerToMove - 1] == false))
                {
                    cout << "That marker cannot be moved! Choose another one: ";
                    cin >> markerToMove;
                    if (markerToMove == 0)
                    {
                        end = true;
                    }
                }
                if (markerToMove > 4)
                {
                    cout << "That marker does not exist! Choose another one: ";
                    cin >> markerToMove;
                    if (markerToMove == 0)
                    {
                        end = true;
                    }
                }
            }
        }
        markerToMove--;
    }
    if ((!end) && (markerCan != 0))
    {
        space = game.players[game.playerTurn].markers[markerToMove];

        if (canMove(game, markerToMove, space))
        {
            move(game, markerToMove, space);

            game.lastMarkerMoved = markerToMove;
        }
        else
        {
            cout << "No marker can be moved!" << endl;
        }
        universalPause();

        if (((game.award > 0) || ((game.roll >= 6) && (game.roll < 9))) || ((game.award > 0) && (game.sixes > 0))) // If a reward is won, or it has just been played a 6/7 or a reward and the number of sixes is > 0, the function will return false
        {
            ok = false;
        }

        else
        {
            game.sixes = 0;
            ok = true;
        }
    }
    return ok;
}

bool canMove(tGame &game, int marker, int &space) // Tell if the marker from the playerTurn can advance roll spaces
{
    bool ok = false;
    int initSpace = space;
    int markersAtPath = 0;

    // It the marker can be moved, the function will return true and the destination space; if it cannot be moved, the function will return false

    if ((game.players[game.playerTurn].markers[marker] == -1) || (game.players[game.playerTurn].markers[marker] == 108)) // The marker will not be able to take a step when it is at the goal or at home
    {
        ok = false;
    }
    else
    {
        for (int i = 0; i < NUM_MARKERS; i++) // In the path to the goal there can only be two markers
        {
            if ((game.players[game.playerTurn].markers[i] > 67) && (game.players[game.playerTurn].markers[i] != 108))
            {
                markersAtPath++;
            }
        }

        for (int i = 1; i < game.roll + 1; i++) // By means of a loop we will try to pass to the next space and if we succeed, we will write down one more movement
        {
            if (space == zanataSpace(game.playerTurn)) // If the current space is the zanata of the playerTurn,the next space is 101
            {
                if (markersAtPath > 1)
                {
                    ok = false;
                }
                else
                {
                    ok = true;
                    space = 101;
                }
            }

            else if (space > 100) // If the current space is in the path to the goal, we just increase the space to go to the next one
            {
                if (space + 1 < 109)
                {
                    ok = true;
                    space = space + 1;
                }
                else
                {
                    ok = false;
                    space = initSpace;
                    i = game.roll + 1;
                }
            }

            else if ((space < 68) && (space > -1)) // If the current space is on the street, we increase the space to go to the next one, but the next one to 67 is 0
            {
                if ((space == 67) && ((game.spaces[0].lane1 == None) || (game.spaces[0].lane2 == None)))
                {
                    ok = true;
                    space = 0;
                }

                else if ((game.spaces[space + 1].lane1 == None) || (game.spaces[space + 1].lane2 == None))
                {
                    ok = true;
                    space = space + 1;
                }

                else if ((game.spaces[space + 1].lane1 != game.spaces[space + 1].lane2))
                {

                    ok = true;
                    space = space + 1;
                }

                else
                {
                    ok = false;
                    space = initSpace;
                    i = game.roll + 1;
                }
            }
        }
        if((space < 68) && (space > -1))
        {
        if ((isSafe(space)) && ((game.spaces[space].lane1 != None) && (game.spaces[space].lane2 != None)))
        {
            ok = false;
            space = initSpace;
        }
        }
    }

    return ok;
}

void move(tGame &game, int marker, int space) // Moves the marker of the playerTurn to the space, perhaps getting a reward for eating a marker or reaching the goal
{

    if ((game.spaces[game.players[game.playerTurn].markers[marker]].lane2 == game.spaces[game.players[game.playerTurn].markers[marker]].lane1) && (game.spaces[game.players[game.playerTurn].markers[marker]].lane2 != None)) // The player has a marker on both lanes or both are empty
    {
        game.spaces[game.players[game.playerTurn].markers[marker]].lane2 = None;
        game.spaces[game.players[game.playerTurn].markers[marker]].lane1 = game.playerTurn;
    }

    else if ((game.spaces[game.players[game.playerTurn].markers[marker]].lane2 != game.spaces[game.players[game.playerTurn].markers[marker]].lane1) && (game.spaces[game.players[game.playerTurn].markers[marker]].lane2 != None)) // On both lanes there is a marker but not from the same player
    {
        if (game.spaces[game.players[game.playerTurn].markers[marker]].lane2 == game.playerTurn)
        {
            game.spaces[game.players[game.playerTurn].markers[marker]].lane2 = None;
        }
        else
        {
            game.spaces[game.players[game.playerTurn].markers[marker]].lane1 = game.spaces[game.players[game.playerTurn].markers[marker]].lane2;
            game.spaces[game.players[game.playerTurn].markers[marker]].lane2 = None;
        }
    }

    else
    {
        game.spaces[game.players[game.playerTurn].markers[marker]].lane2 = None;
        game.spaces[game.players[game.playerTurn].markers[marker]].lane1 = None;
    }

    game.players[game.playerTurn].markers[marker] = space;

    if (space == 108) // A marker has game.award = 10;arrived to the goal. The player gets a reward
    {
        game.award = 10;
        game.awardWon = true;
        cout << "The " << colorToStr(game.playerTurn) << " marker has arrived to the goal! Reward: 10 extra movements" << endl;
        universalPause();
    }
    else if (space < NUM_SPACES)
    {
        if (game.spaces[space].lane1 == None)
        {
            game.spaces[space].lane1 = game.playerTurn;
        }
        else if (game.spaces[space].lane1 == game.playerTurn)
        {
            game.spaces[space].lane2 = game.playerTurn;
        }
        else if ((game.spaces[space].lane1 != game.playerTurn) && (!isSafe(space))) // Eating a marker
        {
            game.spaces[space].lane2 = game.spaces[space].lane1;
            game.spaces[space].lane1 = game.playerTurn;

            toHome(game, space);
            cout << "A marker was eaten! Reward: 20 extra movements" << endl;
            game.award = 20;
            game.awardWon = true;
        }
        else if ((isSafe(space)) && (game.spaces[space].lane1 != None))
        {
            game.spaces[space].lane2 = game.playerTurn;
        }
    }
}
