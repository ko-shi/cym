#ifndef CYM_PARSER_HPP
#define CYM_PARSER_HPP

#include<cym/CymBase.hpp>
#include<cym/utils/CymVector.hpp>
#include<cym/lang/CymAST.hpp>
#include<cym/parser/CymStringOperation.hpp>

namespace cym {

	class Parser {
	public:
		enum Kind{
			FUNCTION,
			PARAM,
			INFIX,
			RESTRICTION,

		};
	private:
		struct Scope {
			Vector<StrView> infixes;
			Vector<StrView> restrictions;
		};
		Vector<Scope> scope_;
		Vector<Str> code_;

		void addCode(const Str &code) {
			const auto head = removeSpace(code);
			if (!head.empty() && head[0] == u'/') {
				if (head.size() <= 1 || (head.size() > 1 && head[1] != u'/')) {
					if (!code_.empty()) {
						code_.back() += Str(head);
					}
					else {
						// ÉGÉâÅ[ : ç≈èâÇÃçsÇ≈/
					}
				}
			}
			code_.emplace_back(code);
		}
	};

}

#endif
