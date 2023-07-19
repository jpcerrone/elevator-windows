# pragma once
struct Vector2i {
    union {
        int x;
        int width;
    };
    union {
        int y;
        int height;
    };
};
