#ifndef CYM_EMBODIED_CODE_GENERATOR
#define CYM_EMBODIED_CODE_GENERATOR

#include<cym/lang/CymIntermediateCodeGenerator.hpp>
#include<cym/utils/CymTree.hpp>
/*
namespace cym {
	struct IntTag {};
	
	struct TypeTag {
		using TypeInfo = Variant<
			IntTag,
			Vector<TypeTag*>, // クラスの時、メンバー変数の型
			TypeTag* // 配列の時、配列の型
		>;
		TypeInfo type;
	};
	class ECode {
		using ScopeKind = ICode::ScopeKind;
		Tree icode_;
		ECode(Tree &&t) : icode_(std::move(t)) {

		}
		void embody() {
			Vector<Pair<Tree*,ScopeKind>> scope;
			const auto &main_order = icode_.get<Tree::ObjectType>()[u"Order"]->get<Tree::ArrayType>();
			scope.emplaceBack(&icode_,ScopeKind::DEFINING_FUNC);
			for (auto && order : main_order) {
				if (order->getWhichHas() != TreeTypes::OBJECT) {
					return;
				}
				const auto kind = order->get<Tree::ObjectType>()[u"Kind"]->get<Str>();
				if (kind == u"DefineParam") {
					order->get<Tree::ObjectType>()[u"Cons"];
				}
			}
		}
		void infer(Tree *expr,const Vector<Pair<Tree*, ScopeKind>> &scope) {
			switch (expr->getWhichHas()) {
			case TreeTypes::NUMBER:
				break;
			case TreeTypes::OBJECT:
				const auto kind = expr->get<Tree::ObjectType>()[u"Kind"]->get<Str>();
				if (kind == u"Func") {
					const auto name = expr->get<Tree::ObjectType>()[u"Name"]->get<Str>();
					const auto args = expr->get<Tree::ObjectType>()[u"Args"]->get<Tree::ArrayType>();
					for (auto &&i : args) {
						infer(i.get(), scope);
					}

					for (auto itr = scope.end() - 1; itr >= scope.begin(); itr--) {
						switch (itr->second) {
						case ScopeKind::DEFINING_FUNC:
							const auto &funcs = itr->first->get<Tree::ObjectType>()[u"DefinedFunc"]->get<Tree::ObjectType>()[name]->get<Tree::ArrayType>();
							for (auto &&i : funcs) {
								i->get<Tree::ObjectType>()[u"Restriction"]
							}
						}
					}
				}
				
				break;
			}
		}

	};
}
*/

#endif