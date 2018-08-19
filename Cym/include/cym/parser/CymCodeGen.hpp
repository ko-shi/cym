#ifndef CYM_CODE_GENERATOR
#define CYM_CODE_GENERATOR

#include<cym/CymBase.hpp>
#include<cym/lang/CymAST.hpp>
#include<cym/vm/CymOpcode.hpp>
#include<cym/lang/CymInstancedType.hpp>
#include<cym/utils/CymForwardIndexMap.hpp>
#include<cym/utils/CymDoubleKeyMap.hpp>

namespace cym {
	class CodeGen {
		ByteCode byte_code_;
		FuncDef ast_;
		Map<InstancedType,Str> inst_type_;
		DoubleKeyMap<FuncIdentifier,Pair<const InstancedType*,ByteCodeFunc>> defined_operators_;
		CodeGen(FuncDef &&a) : ast_(std::move(a)) {
			// primitive type
			inst_type_.emplace(u"Void", VoidType());
			inst_type_.emplace(u"Int", IntType());
			const auto int_type = findType(IntType());


			addOperator(FuncIdentifier(u"+", nullptr, { int_type,int_type }), int_type, ByteCodeFunc{ {
					OpUnion(OpPushVariable{ 0 }),
					OpUnion(OpPushVariable{ 1 }),
					OpUnion(OpBinaryOp{BinOp::PLUS}),
					OpUnion(OpCall{}),
					OpUnion(OpReturnBinaryOp{})
				} ,2});
		}
		template<class T>
		InstancedType* findType(T && type) {
			return &inst_type_.emplace(std::forward<T>(type)).first;
		}

		void addOperator(const FuncIdentifier &funcid, const InstancedType* ret_t,const ByteCodeFunc &code) {
			defined_operators_.emplace(funcid, makePair(ret_t,code));
		}
		const Vector<FuncDef*> findFunc(const Vector<Scope> &s,const Str &name, const Vector<InstancedType*> &arg_types) {
			// TODO: オーバーロード対応
			auto list = Vector<FuncDef*>();
			for (auto i = s.rbegin(); i != s.rend(); i++) {
				if (i->index() == ScopeKind::FUNC_SCOPE) {
					const auto &func = std::get<ScopeKind::FUNC_SCOPE>(*i);
					if (func->inner_func.count(name)) {
						const auto &found = func->inner_func[name];
						for (const auto &arg : found.args) {
							//arg.second
						}
						// narrow
						list.push_back(&func->inner_func[name]);
					}
				}
			}
			return list;
		}
		void generate() {
			const auto void_type = findType(VoidType());
			const auto main = doGenerate(FuncIdentifier(u"main@", &ast_, {void_type}), { &ast_ });
			//addOperator(u"main@", FunctionType{ void_type,{} });
		}
		Pair<InstancedType*,ByteCode> doGenerate(const FuncIdentifier &funcid, Vector<Scope> stratum) {// copy cnstruct
			
			for (const auto &i : funcid.def->order) {
				consignAST(i.get(), stratum);
			}
		}
		Pair<InstancedType*,ByteCodeFunc> consignAST(ASTBase *ast,const Vector<Scope> &stratum) {// return value is returrn calue
			switch (ast->id()) {
			case ASTId::CALL_FUNC: {
				const auto func = dynamic_cast<ASTCallFunc*>(ast);
				genCallFunc(stratum, *func);
			}
			case ASTId::INFIX: {
				const auto infix = dynamic_cast<ASTInfix*>(ast);
				infix->l
			}
			}
		}
		Vector<Pair<InstancedType*,ByteCodeFunc>> genCallFunc(const Vector<Scope> &stratum, const ASTCallFunc &ast) {
			Vector<InstancedType*> args(ast.args.size());
			Vector<ByteCodeFunc> ops(ast.args.size());
			for (auto &arg : ast.args) {
				const auto arg_code = consignAST(arg.get(), stratum);
				args.emplace_back(arg_code.first);
				ops.emplace_back(arg_code.second);
			}
			
			//defined_operators_[FuncIdentifier{n}]

			auto list = Vector<Pair<InstancedType*, ByteCodeFunc>>();
			const auto func = findFunc(stratum, ast.name, args);
			for (const auto f : func) {
				const auto id = FuncIdentifier(ast.name, f, args);
				if (defined_operators_.link.count(id)) {
					defined_operators_[id]
				}
				else {

				}
			}
		}
	};
}

#endif