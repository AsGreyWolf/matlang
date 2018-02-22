#include "parser.hpp"

using namespace std;

int main() {
	matlang::parser prsr;
	string str;
	while (cout << "> ", getline(cin, str)) {
		try {
			matlang::object result;
			if (prsr.eval(0, str, result) != str.size())
				throw matlang::parse_error(str.size(), "unhandled error");
			cout << result << endl;
			prsr.clear();
		} catch (std::bad_variant_access &) {
			cerr << "Type mismatch" << endl;
		} catch (matlang::parse_error &e) {
			for (int i = 0; i < e.i_ + 2; i++)
				cerr << ' ';
			cerr << '^' << endl;
			cerr << e.what() << endl;
		} catch (std::exception &e) {
			cerr << e.what() << endl;
		}
	}
}
