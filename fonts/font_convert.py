##
# https://www.pygame.org/docs/ref/image.html
# https://www.pygame.org/docs/ref/font.html
from pygame.font import Font
from pygame import image, init
import sys
import os

RENDER_CHARS = [chr(x) for x in range(32, 127)]
RENDER_STRING = "".join(RENDER_CHARS)


def main():
    """
    use pygame to convert a TTF to BMP
    """
    if len(sys.argv) < 3:
        print("Usage: {} <TTF-file> <size>".format(sys.argv[0]))
        exit(1)

    fname = sys.argv[1]
    fsize = int(sys.argv[2])

    outname = "{}_{}.BMP".format(os.path.splitext(fname)[0], fsize)

    print("Loading font {} in height {}...".format(fname, fsize))
    init()
    f = Font(fname, fsize)

    print("Rendering {} characters '{}'".format(len(RENDER_STRING), RENDER_STRING))
    width = None
    for ch in RENDER_CHARS:
        w, h = f.size(str(ch))
        if width is None:
            width = w
        if w != width:
            print("ERROR: Font is not monospaced!")
            exit(1)

    surface = f.render(RENDER_STRING, False, (255, 255, 255), (0, 0, 0))

    print("Writing rendered characters to '{}'...".format(outname))
    image.save(surface, outname)


if __name__ == "__main__":
    # execute only if run as a script
    main()

