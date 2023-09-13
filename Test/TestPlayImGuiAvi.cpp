/// Player header
#include "PlayAvi.h"

int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        printf("Usage: %s <input-file> \n", argv[0]);
        return EXIT_SUCCESS;
    }
    std::string fileName(argv[1]);
    float       freq = 18.0;
    if (argc == 3)
    {
        freq = atof(argv[2]);
    }
    std::shared_ptr<ImGui::PlayAvi> run(new ImGui::PlayAvi(fileName, freq));
    return EXIT_SUCCESS;
}
