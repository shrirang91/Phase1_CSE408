#ifndef COLORINSTANCE_H
#define COLORINSTANCE_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

class ColorInstance
{
public:

    typedef std::vector<ColorInstance *> ColorVector;

    ColorInstance(std::string type) {
        _type = type;
    }

    ColorInstance(std::string type, float c1, float c2, float c3) {
        _type = type;
        _c1 = c1;
        _c2 = c2;
        _c3 = c3;
    }

    std::string getType();
    void readInstance();
    ColorVector interpolate(ColorInstance *other, int partitions);

    std::string toString();

    static ColorInstance *makeInstance(std::string type);

    static const std::string COLOR_RGB;
    static const std::string COLOR_XYZ;
    static const std::string COLOR_LAB;
    static const std::string COLOR_YUV;
    static const std::string COLOR_YCBCR;
    static const std::string COLOR_YIQ;
    static const std::string COLOR_HSL;

    static const std::string COLOR_SPACES[];
    static const int NUM_COLOR_SPACES;

    std::string _type;
    float _c1;
    float _c2;
    float _c3;
};

#endif // COLORINSTANCE_H
