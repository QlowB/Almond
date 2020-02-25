#include "MandelVideoGenerator.h"
#include "Gradient.h"
#include "Mandel.h"
#include "Fixed.h"


int main() {
    //mnd::MandelContext mndCtxt = mnd::initializeContext();


    ExportVideoInfo evi;
    
    evi.start = mnd::MandelViewport::standardView();
    evi.end = mnd::MandelViewport {
        mnd::Real("-1.5016327722130767973008541252724123393337183519056236025189105693015282429244791506194548898968185999262221668435271537932672968559900159142085320685031"),
        mnd::Real("9.1949171527697821768939276268368163504538591789778359909730511642378316080598664365235178721745031546786105261407973733873085119833457073054327967448264e-06"),
        mnd::Real("1.6236294899543021550377844129369984149698872979955210084321757728274664401182171658849308001321609757279087031477100527629814577654596624031152718524352e-28"),
        mnd::Real("1.2246019034401093377903721086780361028058704962292211685926779200766324399350798858672587301860274703389823933260119617558370004128301410779021141722617e-28")
    };
    //evi.end.zoomCenter(1.0e+27);
    evi.gradient = Gradient::defaultGradient();

    evi.width = 2560;
    evi.height = 1440;
    evi.maxIterations = 20000;
    evi.fps = 60;
    evi.zoomSpeed = 0.9;
    evi.path = "video.avi";
    evi.bitrate = 15000;
    evi.preset = "slow";

    evi.start.adjustAspectRatio(evi.width, evi.height);

    MandelVideoGenerator mvg(evi);

    mvg.generate();
    return 0;
}


