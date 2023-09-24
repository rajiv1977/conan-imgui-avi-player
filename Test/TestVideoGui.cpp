#include "Video.h"

int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        printf("Usage: %s <input-file> \n", argv[0]);
        return EXIT_SUCCESS;
    }
    std::string fileName(argv[1]);

    std::shared_ptr<Gui::Video> run(new Gui::Video(fileName));

    return EXIT_SUCCESS;
}
