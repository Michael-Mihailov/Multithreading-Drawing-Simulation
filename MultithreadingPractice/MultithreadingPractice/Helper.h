#include <raylib.h>
#ifndef HELPER_H
#define HELPER_H


namespace CPSC30000
{
    class Cell
    {
        // Technically isn't needed here as variables default to private unless stated to be public
    private:
        int _posX;
        int _posY;
        int _width;
        int _height;
        Color _color;

    public:
        Cell(int posX, int posY, int width, int height, Color color)
            : _posX(posX),
            _posY(posY),
            _width(width),
            _height(height),
            _color(color)
        {
        }
        Cell()
        { }


        void SetVariables(int posX, int posY, int width, int height, Color color)
        {
            _posX = posX;
            _posY = posY;
            _width = width;
            _height = height;
            _color = color;
        }

        void ChangeColor(Color color)
        {
            _color = color;
        }

        void Draw()
        {
            DrawRectangle(_posX, _posY, _width, _height, _color);
        }
    };
}


#endif // !HELPER_H
