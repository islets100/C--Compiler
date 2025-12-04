# 编译器设置
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# 目录设置
LEX_DIR = lex
GRAMMAR_DIR = Syntax

# 目标文件
LEX_EXE = $(LEX_DIR)/Lex_Analysis.exe
GRAMMAR_EXE = $(GRAMMAR_DIR)/Syntaxer.exe

# 默认目标
all: lex grammar

# 编译词法分析器
lex:
	cd $(LEX_DIR) && $(CXX) $(CXXFLAGS) Lex_Analysis.cpp -o Lex_Analysis.exe

# 编译语法分析器
grammar:
	cd $(GRAMMAR_DIR) && $(CXX) $(CXXFLAGS) main.cpp GrammarAnalyzer.cpp SLRTable.cpp Parser.cpp ReductionSequenceLogger.cpp -o Syntaxer.exe

# 清理
clean:
	cd $(LEX_DIR) && del /Q Lex_Analysis.exe 2>nul || rm -f Lex_Analysis.exe
	cd $(GRAMMAR_DIR) && del /Q Syntaxer.exe 2>nul || rm -f Syntaxer.exe

# 运行词法分析器
run-lex: lex
	cd $(LEX_DIR) && Lex_Analysis.exe

# 运行语法分析器（需要先运行词法分析器）
run-grammar: grammar
	cd $(GRAMMAR_DIR) && Syntaxer.exe

# 运行完整编译链（一次性运行词法和语法分析）
run-all: lex
	@echo Running lexical analysis...
	cd $(LEX_DIR) && Lex_Analysis.exe
	@echo Running syntax analysis...
	cd ../$(GRAMMAR_DIR) && Syntaxer.exe

.PHONY: all lex grammar clean run-lex run-grammar run-all