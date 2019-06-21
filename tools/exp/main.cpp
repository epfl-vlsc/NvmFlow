#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IRReader/IRReader.h"

#include <bitset>
#include <memory>
#include <string>

#include "Common.h"

using namespace llvm;
using std::string;
using std::unique_ptr;

static cl::OptionCategory propagationCategory { "persistent pointer options" };

static cl::opt<string> bitcodeFile { cl::Positional,
		cl::desc { "<Module to analyze>" },
		cl::value_desc { "bitcode filename" }, cl::init(""), cl::Required,
		cl::cat { propagationCategory } };

int main(int argc, char** argv) {
	// This boilerplate provides convenient stack traces and clean LLVM exit
	// handling. It also initializes the built in support for convenient
	// command line option handling.
	sys::PrintStackTraceOnErrorSignal(argv[0]);
	llvm::PrettyStackTraceProgram X(argc, argv);
	llvm_shutdown_obj shutdown;
	cl::HideUnrelatedOptions(propagationCategory);
	cl::ParseCommandLineOptions(argc, argv);

	// Construct an IR file from the filename passed on the command line.
	SMDiagnostic err;
	LLVMContext context;
	std::string& bcFile = bitcodeFile.getValue();
	unique_ptr < Module > module = parseIRFile(bcFile, err, context);
	if (!module.get()) {
		errs() << "Error reading bitcode file: " << bcFile << "\n";
		err.print(argv[0], errs());
		return -1;
	}

	Module *M = module.release();

	return 0;
}