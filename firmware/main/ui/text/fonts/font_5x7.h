#pragma once

#include <cstdint>

// ------------------------------------------------------------
// 5x7 BITMAP FONT
// ------------------------------------------------------------
//
// Each glyph contains 5 columns and 7 rows.
//
// Bit 0 = top pixel
// Bit 6 = bottom pixel
//
// The font is intentionally compact and monochrome.
// The renderer scales the glyphs for larger text sizes.
//
// ------------------------------------------------------------

namespace Font5x7
{
    constexpr uint8_t GLYPH_WIDTH =
        5;

    constexpr uint8_t GLYPH_HEIGHT =
        7;

    constexpr uint8_t GLYPH_SPACING =
        1;

    // --------------------------------------------------------
    // SPACE
    // --------------------------------------------------------

    constexpr uint8_t SPACE[5] =
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00
    };

    // --------------------------------------------------------
    // NUMBERS
    // --------------------------------------------------------

    constexpr uint8_t ZERO[5] =
    {
        0x3E,
        0x51,
        0x49,
        0x45,
        0x3E
    };

    constexpr uint8_t ONE[5] =
    {
        0x00,
        0x42,
        0x7F,
        0x40,
        0x00
    };

    constexpr uint8_t TWO[5] =
    {
        0x62,
        0x51,
        0x49,
        0x49,
        0x46
    };

    constexpr uint8_t THREE[5] =
    {
        0x22,
        0x49,
        0x49,
        0x49,
        0x36
    };

    constexpr uint8_t FOUR[5] =
    {
        0x18,
        0x14,
        0x12,
        0x7F,
        0x10
    };

    constexpr uint8_t FIVE[5] =
    {
        0x2F,
        0x49,
        0x49,
        0x49,
        0x31
    };

    constexpr uint8_t SIX[5] =
    {
        0x3E,
        0x49,
        0x49,
        0x49,
        0x32
    };

    constexpr uint8_t SEVEN[5] =
    {
        0x01,
        0x71,
        0x09,
        0x05,
        0x03
    };

    constexpr uint8_t EIGHT[5] =
    {
        0x36,
        0x49,
        0x49,
        0x49,
        0x36
    };

    constexpr uint8_t NINE[5] =
    {
        0x26,
        0x49,
        0x49,
        0x49,
        0x3E
    };

    // --------------------------------------------------------
    // LETTERS
    // --------------------------------------------------------

    constexpr uint8_t A[5] =
    {
        0x7E,
        0x11,
        0x11,
        0x11,
        0x7E
    };

    constexpr uint8_t B[5] =
    {
        0x7F,
        0x49,
        0x49,
        0x49,
        0x36
    };

    constexpr uint8_t C[5] =
    {
        0x3E,
        0x41,
        0x41,
        0x41,
        0x22
    };

    constexpr uint8_t D[5] =
    {
        0x7F,
        0x41,
        0x41,
        0x22,
        0x1C
    };

    constexpr uint8_t E[5] =
    {
        0x7F,
        0x49,
        0x49,
        0x49,
        0x41
    };

    constexpr uint8_t F[5] =
    {
        0x7F,
        0x09,
        0x09,
        0x09,
        0x01
    };

    constexpr uint8_t G[5] =
    {
        0x3E,
        0x41,
        0x49,
        0x49,
        0x7A
    };

    constexpr uint8_t H[5] =
    {
        0x7F,
        0x08,
        0x08,
        0x08,
        0x7F
    };

    constexpr uint8_t I[5] =
    {
        0x00,
        0x41,
        0x7F,
        0x41,
        0x00
    };

    constexpr uint8_t J[5] =
    {
        0x20,
        0x40,
        0x41,
        0x3F,
        0x01
    };

    constexpr uint8_t K[5] =
    {
        0x7F,
        0x08,
        0x14,
        0x22,
        0x41
    };

    constexpr uint8_t L[5] =
    {
        0x7F,
        0x40,
        0x40,
        0x40,
        0x40
    };

    constexpr uint8_t M[5] =
    {
        0x7F,
        0x02,
        0x0C,
        0x02,
        0x7F
    };

    constexpr uint8_t N[5] =
    {
        0x7F,
        0x04,
        0x08,
        0x10,
        0x7F
    };

    constexpr uint8_t O[5] =
    {
        0x3E,
        0x41,
        0x41,
        0x41,
        0x3E
    };

    constexpr uint8_t P[5] =
    {
        0x7F,
        0x09,
        0x09,
        0x09,
        0x06
    };

    constexpr uint8_t Q[5] =
    {
        0x3E,
        0x41,
        0x51,
        0x21,
        0x5E
    };

    constexpr uint8_t R[5] =
    {
        0x7F,
        0x09,
        0x19,
        0x29,
        0x46
    };

    constexpr uint8_t S[5] =
    {
        0x46,
        0x49,
        0x49,
        0x49,
        0x31
    };

    constexpr uint8_t T[5] =
    {
        0x01,
        0x01,
        0x7F,
        0x01,
        0x01
    };

    constexpr uint8_t U[5] =
    {
        0x3F,
        0x40,
        0x40,
        0x40,
        0x3F
    };

    constexpr uint8_t V[5] =
    {
        0x1F,
        0x20,
        0x40,
        0x20,
        0x1F
    };

    constexpr uint8_t W[5] =
    {
        0x7F,
        0x20,
        0x18,
        0x20,
        0x7F
    };

    constexpr uint8_t X[5] =
    {
        0x63,
        0x14,
        0x08,
        0x14,
        0x63
    };

    constexpr uint8_t Y[5] =
    {
        0x03,
        0x04,
        0x78,
        0x04,
        0x03
    };

    constexpr uint8_t Z[5] =
    {
        0x61,
        0x51,
        0x49,
        0x45,
        0x43
    };

    // --------------------------------------------------------
    // SYMBOLS
    // --------------------------------------------------------

    constexpr uint8_t COLON[5] =
    {
        0x00,
        0x36,
        0x36,
        0x00,
        0x00
    };

    constexpr uint8_t DASH[5] =
    {
        0x08,
        0x08,
        0x08,
        0x08,
        0x08
    };

    constexpr uint8_t DOT[5] =
    {
        0x00,
        0x60,
        0x60,
        0x00,
        0x00
    };

    constexpr uint8_t SLASH[5] =
    {
        0x60,
        0x10,
        0x08,
        0x04,
        0x03
    };

    constexpr uint8_t PERCENT[5] =
    {
        0x63,
        0x13,
        0x08,
        0x64,
        0x63
    };

    // --------------------------------------------------------
    // GET GLYPH
    // --------------------------------------------------------

    inline const uint8_t* get(
        char character)
    {
        switch (character)
        {
            case ' ':
                return SPACE;

            case '0':
                return ZERO;

            case '1':
                return ONE;

            case '2':
                return TWO;

            case '3':
                return THREE;

            case '4':
                return FOUR;

            case '5':
                return FIVE;

            case '6':
                return SIX;

            case '7':
                return SEVEN;

            case '8':
                return EIGHT;

            case '9':
                return NINE;

            case 'A':
                return A;

            case 'B':
                return B;

            case 'C':
                return C;

            case 'D':
                return D;

            case 'E':
                return E;

            case 'F':
                return F;

            case 'G':
                return G;

            case 'H':
                return H;

            case 'I':
                return I;

            case 'J':
                return J;

            case 'K':
                return K;

            case 'L':
                return L;

            case 'M':
                return M;

            case 'N':
                return N;

            case 'O':
                return O;

            case 'P':
                return P;

            case 'Q':
                return Q;

            case 'R':
                return R;

            case 'S':
                return S;

            case 'T':
                return T;

            case 'U':
                return U;

            case 'V':
                return V;

            case 'W':
                return W;

            case 'X':
                return X;

            case 'Y':
                return Y;

            case 'Z':
                return Z;

            case ':':
                return COLON;

            case '-':
                return DASH;

            case '.':
                return DOT;

            case '/':
                return SLASH;

            case '%':
                return PERCENT;

            default:
                return SPACE;
        }
    }
}