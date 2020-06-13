#include "run.h"
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/optional.hpp>

namespace progOpts = boost::program_options;

int main(int argc, char** argv)
{
    progOpts::options_description desc("Available options");
    progOpts::positional_options_description p;

    boost::optional<std::string> inPath;
    boost::optional<std::string> outPath;
    desc.add_options()
        ("help", "display this help message")
        ("list-generators", "list all available generators on this machine")
        ("benchmark-generators", "benchmark all available generators on this machine")
        ("render-image,i", "render a mandelbrot view")
        ("file,f", progOpts::value(&inPath), "specifies a file to load")
        ("output,o", progOpts::value(&outPath), "file to output")
    ;


    p.add("file", 1);
    progOpts::variables_map vm;
    progOpts::store(progOpts::command_line_parser(argc, argv)
                .options(desc).positional(p).run(), vm);
    progOpts::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }


    if (vm.count("list-generators")) {
        mnd::MandelContext mndCtxt = mnd::initializeContext();
        listGenerators(mndCtxt);
    }
    else if (vm.count("benchmark-generators")) {
        mnd::MandelContext mndCtxt = mnd::initializeContext();
        benchGenerators(mndCtxt);
    }
    else if (vm.count("render-image")) {
        if (!inPath) {
            std::cout << "Please specify a path" << std::endl;
            return 1;
        }
        if (!outPath) {
            std::cout << "Please specify an output file" << std::endl;
            return 1;
        }
        renderImage(*inPath, *outPath);
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


