#include "run.h"
#include <iostream>

#include <boost/program_options.hpp>

namespace progOpts = boost::program_options;

int main(int argc, char** argv)
{
    progOpts::options_description desc("Available options");
    desc.add_options()
        ("help", "display this help message")
        ("render-image,i", "render a mandelbrot view")
        ("file,f", progOpts::value<std::string>(), "specifies a file to load")
        ("output,o", progOpts::value<std::string>(), "file to output")
    ;
    progOpts::positional_options_description p;
    p.add("file", 1);
    progOpts::variables_map vm;
    progOpts::store(progOpts::command_line_parser(argc, argv)
                .options(desc).positional(p).run(), vm);
    progOpts::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    std::string out = vm["output"].as<std::string>();

    if (vm.count("render-image")) {
        std::string inPath = vm["file"].as<std::string>();
        std::cout << "rendering image " << inPath << std::endl;
        renderImage(inPath, out);
    } else {
        std::cout << "No files specified" << std::endl;
    }

/*
    mnd::MandelContext mndCtxt = mnd::initializeContext();


    ExportVideoInfo evi;
    
    evi.start = mnd::MandelViewport::standardView();
    evi.end = mnd::MandelViewport {
        mnd::Real("-1.0"),
        mnd::Real("-1.0"),
        mnd::Real("1.0e-3"),
        mnd::Real("1.0e-3")
    };
    //evi.end.zoomCenter(1.0e+27);
    evi.gradient = Gradient::defaultGradient();

    

    mnd::MandelContext mc = mnd::initializeContext();
    mnd::MandelInfo mi;
    mi.view = evi.start;
    mi.bWidth = 12000;
    mi.bHeight = 12000;
    mi.maxIter = 800;
    mi.smooth = true;
    alm::ImageExportInfo iei;
    iei.drawInfo = mi;
    iei.gradient = evi.gradient;
    iei.generator = &mc.getDefaultGenerator();
    iei.options.jpegQuality = 100;
    iei.path = "file.jpg";
    try {
        alm::exportImage(iei);
    } catch (alm::ImageExportException& iee) {
        printf("%s\n", iee.what());
        return 1;
    }

    return 0;
    */
}


