#include <pxr/pxr.h>
#include <pxr/base/tf/diagnosticMgr.h>

using namespace PXR_NS;

int main(int argc, char* argv[])
{
    TfDiagnosticMgr::GetInstance().SetQuiet(true);
    return 0;
}
