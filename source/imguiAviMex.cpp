
/// Matlab C++ API
#include "mex.hpp"
#include "mexAdapter.hpp"

/// Player header
#include "PlayAvi.h"

/// Current Matlab OpenGL AVI version
#define MATLAB_OPENGL_AVI_VERSION 1

class MexFunction : public matlab::mex::Function
{
  private:
    /// Factory helper for creating Matlab arrays
    matlab::data::ArrayFactory mFactory;

    /// Handle to Matlab engine
    std::shared_ptr<matlab::engine::MATLABEngine> mMatlabPtr = getEngine();

    /// Helper functions
    bool validateArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs);

    void displayError(std::string errorMessage);

    template <class T, class U>
    U dataFormat(T& data)
    {
        T object(data);
        return U(object.begin(), object.end());
    }

    void toLower(std::string& data);

  public:
    /// main entry point
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs);
};

void MexFunction::operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
{
    // Check to verify the validity of the Matlab’s input
    if (validateArguments(outputs, inputs))
    {
        matlab::data::TypedArray<matlab::data::MATLABString> inputInfo = inputs[0];
        auto                                                 fileName =
            dataFormat<matlab::data::TypedArray<matlab::data::MATLABString>, std::vector<std::string>>(inputInfo);
        float freq = 18.0;
        if (inputs.size() == 2)
        {
            matlab::data::TypedArray<double> freqInfo = inputs[1];
            freq = dataFormat<matlab::data::TypedArray<double>, std::vector<double>>(freqInfo)[0];
        }
        std::shared_ptr<ImGui::PlayAvi> run(new ImGui::PlayAvi(fileName[0], freq));
    }
}

bool MexFunction::validateArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
{
    bool status = true;

    // Verify the input
    if (inputs.size() != 0 && inputs.size() <= 2)
    {
        if (inputs[0].getType() != matlab::data::ArrayType::MATLAB_STRING)
        {
            status               = false;
            std::string errorMsg = "Input should be a string";
            displayError(errorMsg);
        }
        else
        {
            matlab::data::TypedArray<matlab::data::MATLABString> internalInput = inputs[0];
            auto fileName = dataFormat<matlab::data::TypedArray<matlab::data::MATLABString>, std::vector<std::string>>(
                internalInput);
            if (fileName[0].compare("webcam") != 0)
            {
                std::size_t found = fileName[0].find_last_of(".");
                std::string str   = fileName[0].substr(found + 1, fileName[0].size());
                toLower(str);
                if (!(str.compare("avi") == 0 || str.compare("tavi") == 0))
                {
                    status               = false;
                    std::string errorMsg = ".avi format only.";
                    displayError(errorMsg);
                }
            }
        }

        if (inputs.size() == 2)
        {
            if (inputs[1].getType() != matlab::data::ArrayType::DOUBLE)
            {
                status               = false;
                std::string errorMsg = "Input should be a double";
                displayError(errorMsg);
            }
        }
    }
    else
    {
        status               = false;
        std::string errorMsg = "Only one input and that should be a string";
        displayError(errorMsg);
    }

    // Verify the output conditions.
    if (outputs.size() != 0)
    {
        status               = false;
        std::string errorMsg = "No output arguments.";
        displayError(errorMsg);
    }
    return status;
}

// Helper function to generate an error message from given string,
// and display it over MATLAB command prompt.

void MexFunction::displayError(std::string errorMessage)
{
    mMatlabPtr->feval(u"error", 0, std::vector<matlab::data::Array>({mFactory.createScalar(errorMessage)}));
}

void MexFunction::toLower(std::string& data)
{
    std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) { return std::tolower(c); });
}
