#include <iostream>
#include <fstream>
#include <string>

class Node
{
private:
	std::string m_token;
	Node*       m_left;
	Node*       m_right;
	Node*       m_father;
	double      m_value;

public:
	Node ()             : m_token(""),    m_left(nullptr), m_right(nullptr), m_father(nullptr) {}   // default ctor
	Node (Node* father) : m_token(""),    m_left(nullptr), m_right(nullptr), m_father(father)  {}   // ctor using father

	//setters
	void setToken    (const std::string& key) { m_token = key;     }
	void setLeft     (Node* left)             { m_left = left;     }
	void setRight    (Node* right)            { m_right = right;   }
	void setFather   (Node* father)           { m_father = father; }
	void setValue    (double value)           { m_value = value;   }

	//getters
	const  std::string& token () { return m_token;  }
	Node*  left               () { return m_left;   }
	Node*  right              () { return m_right;  }
	Node*  father             () { return m_father; }
	double value              () { return m_value;  }
};

class Tree
{
private:
	Node* m_root;
	bool isEvaluated;

	//string functions
	void        p_removeChar    (std::string& str, const char c[])
	{
		//---removes all occurances of the characters in c from str---
		for (std::string::size_type i = 0; i < str.size(); ++i)
			for(int j = 0; c[j]; ++j)
				if (str[i] == c[j])
				{
					str.erase(i, 1);
					--i;
					break;
				}
	}
	bool        p_isOperator    (const char c)
	{
		if (c == '+' || c == '-' || c == '*' || c == '/' || c == '~')
			return true;
		return false;
	}
	bool        p_isOperator    (const std::string& str)
	{
		if (str.size() == 1)
			return p_isOperator(str[0]);
		return false;
	}
	bool        p_isValue       (const std::string& str)
	{
		//---check if string is in decimal--- 
		const std::string::size_type size = str.size();
		for (std::string::size_type i = 0; i < str.size(); ++i)
			if ((str[i] < '0' || str[i] > '9') && str[i] != '.')
				return false;
		return true;
	}
	std::string p_stringToWords (const std::string& str, const char sep, const bool isNew, std::string::size_type start = 0)  
	{
		//a generator for words in a string, like strtok

		static std::string::size_type i;
		//reset static iterator
		if (isNew)
			i = start;

		//return empty string if the end is reached
		const std::string::size_type size = str.size();
		if (i >= size - 1 || start >= size - 1)
			return std::string("");

		//exhaust starting separators
		while (i < size && str[i] == sep) ++i;

		//return operator words so that no sep is needed for single digit values 
		if (p_isOperator(str[i]))
			return str.substr(i++, 1);

		//advance until next separator
		const std::string::size_type oldI = i;
		while (i < size && str[i] != sep && !p_isOperator(str[i])) ++i;

		//return the substring
		return str.substr(oldI, i - oldI);
	}

	//tree reccursive functions
	void   r_clear         (Node* start) //deletes the tree
	{
		if (start)
		{
			r_clear(start->left());
			r_clear(start->right());
			delete start;
		}
	}
	void   r_print         (Node* currentNode)
	{
		if (currentNode)
		{
			if (currentNode->right())
			{
				std::cout << '(';
				r_print(currentNode->left());
				std::cout << ' ' << currentNode->token() << ' ';
				r_print(currentNode->right());
				std::cout << ')';
			}
			else if (currentNode->left())
			{
				std::cout << '(';
				std::cout << currentNode->token();
				r_print(currentNode->left());
				std::cout << ')';
			}
			else
				std::cout << currentNode->token();
		}
	}
	void   r_getTreeLeaves (Node* pos, int &leaves)
	{ //counts the space needed for the tree
		if (pos)
		{
			if (!pos->left()) //leafs dont have children
				leaves += pos->token().size() / 4 + 1; //make tree big enough to fit multi-digit vlues
			else if(pos->right()) //move down on both branches
			{
				r_getTreeLeaves(pos->left(), leaves);
				r_getTreeLeaves(pos->right(), leaves);
			}
			else //move down on one branch (for unary operators)
			{
				int saveVal = leaves;
				r_getTreeLeaves(pos->left(), leaves);

				//add the new leaves twice in order to avoid collisions in the char tree matrix 
				leaves += leaves - saveVal;
			}
		}
	}
	void   r_TreeString    (char treeString[256][256], Node* pos, int x, int y)
	{ //builds the tree char matrix
		if (pos)
		{
			if (x >= 0 && y >= 0 && x < 254 && y < 254)
			{
				if (pos->left()) // operators
				{
					treeString[y][x] = pos->token()[0];

					int depth = -1;
					r_getTreeLeaves(pos, depth);

					if (pos->right()) // binary operators
					{
						for (int i = 0; i < depth; i++) //adds verts
						{
							treeString[y + i + 1][x - i - 1] = '/';
							treeString[y + i + 1][x + i + 1] = '\\';
						}
						r_TreeString(treeString, pos->left(), x - depth - 1, y + depth + 1);
						r_TreeString(treeString, pos->right(), x + depth + 1, y + depth + 1);
					}
					else //unary operators
					{
						for (int i = 0; i < depth; i++) //adds vert
						{
							treeString[y + i + 1][x - i - 1] = '/';
						}
						r_TreeString(treeString, pos->left(), x - depth - 1, y + depth + 1);
					}
				}
				else //for operands
				{
					std::string::size_type size = pos->token().size();
					for(int i = 0; i < size; ++i) //center and cast the values into the matrix
						if(size & 1)
							treeString[y][x + i - size / 2] = pos->token()[i];
						else
							treeString[y][x + i - size / 2 + 1] = pos->token()[i];
				}
			}
		}
	}
	double r_evaluate      (Node* start, bool& error)
	{
		if (!error)
		{
			if (start->token() == "+")
			{
				start->setValue(r_evaluate(start->left(), error) + r_evaluate(start->right(), error));
			}
			else if (start->token() == "-")
			{
				start->setValue(r_evaluate(start->left(), error) - r_evaluate(start->right(), error));
			}
			else if (start->token() == "*")
			{
				start->setValue(r_evaluate(start->left(), error) * r_evaluate(start->right(), error));
			}
			else if (start->token() == "/")
			{
				const double val = r_evaluate(start->right(), error);
				if (val)
					start->setValue(r_evaluate(start->left(), error) / val);
				else
				{
					std::cout << "Error! Division by 0\n";
					error = true;
				}
			}
			else if (start->token() == "~")
			{
				start->setValue(-r_evaluate(start->left(), error));
			}
			return start->value();
		}
		return NULL;
	}


public:
	Tree () : m_root(nullptr), isEvaluated(false) {}   //default ctor
	Tree (std::string exp)                             //ctor using an expression
	{
		//remove parenthesis, not needed for prefix notation
		p_removeChar(exp, "()");

		if (exp.size())
		{
			//creating the root node and a sentinel node as its father
			m_root = new Node();
			m_root->setFather(new Node());
			Node* currentNode = m_root;

			//going through each word, ' ' is separator
			bool error = false;
			std::string word = p_stringToWords(exp, ' ', true);
			while (word.size())
			{
				if (currentNode == m_root->father()) //if the sentinel is reached but the expression is not exhausted
				{
					std::cout << "Error! Ill-defined expression, missing operator\n";
					error = true;
					break;
				}
				if (p_isOperator(word)) //operator found: set to current node and move to the left child
				{
					currentNode->setToken(word);
					currentNode->setLeft(new Node(currentNode));
					currentNode = currentNode->left();
				}
				else if (p_isValue(word)) //operand found: set to current node and move up the tree until a node without a right child is found
				{
					currentNode->setToken(word);
					currentNode->setValue(std::stod(word));

					do { currentNode = currentNode->father(); } while (currentNode->father() && (currentNode->right() || currentNode->token() == "~"));

					if (currentNode != m_root->father()) //create the right child unless the current node is the sentinel
					{
						currentNode->setRight(new Node(currentNode));
						currentNode = currentNode->right();
					}
				}
				else
				{
					std::cout << "\nError! Unrecognizable word: \"" << word << "\"\n";
					error = true;
					break;
				}
				word = p_stringToWords(exp, ' ', false);
			}

			if (currentNode != m_root->father()) //the sentinel node must be reached by the end
			{
				std::cout << "Error! Ill-defined expression, missing operand\n";
				error = true;
			}

			if (error) //handle errors
			{
				clear();
				std::cout << "Tree dumped\n";
			}
			else //success, the sentinel can now be deleted and the tree cand be evaluated
			{
				delete m_root->father();
				std::cout << "Tree built successfully\n";
				bool divBy0 = false;
				r_evaluate(m_root, divBy0);
				isEvaluated = !divBy0;
			}
		}
		else
			std::cout << "Error! Empty expression\n";
	}

	~Tree()                        //dtor
	{
		if (m_root)
		{
			r_clear(m_root->left());
			r_clear(m_root->right());
		}
	}

	//getters
	Node*  root  () { return m_root;                          }
	double value () { return (m_root && isEvaluated) ? m_root->value() : NULL; }

	//functionalities
	void clear   () //deletes tree
	{
		r_clear(m_root->left());
		r_clear(m_root->right());
		m_root = nullptr;
	}
	void print   () //prints tree in infix notation
	{
		if (m_root)
		{
			r_print(m_root);
			if (isEvaluated)
				std::cout << " = " << value();
			std::cout<< '\n';
		}
		else
			std::cout << "Tree is empty\n";
	}
	void display ()
	{
		char treeString[256][256];
		for (int i = 0; i < 256; i++)
		{ //create empty char matrix
			for (int j = 0; j < 255; j++)
				treeString[i][j] = ' ';
			treeString[i][255] = 0;
		}

		//start as close to 0 0 as posible 
		int allLeaves = 0;
		r_getTreeLeaves(m_root, allLeaves);
		r_TreeString(treeString, m_root, allLeaves * 3, 0);
		//delete useless spaces
		for (int i = 0; i < 256; i++)
		{
			for (int j = 254; j >= 0; j--)
				if (treeString[i][j] != ' ')
				{
					treeString[i][j + 1] = 0;
					break;
				}
			treeString[i][255] = 0;
		}

		std::cout << '\n';
		for (int i = 0; i < 256; i++)
		{ //print matrix
			bool ok = false;
			for (int j = 0; j < 255; j++)
				if (treeString[i][j] != ' ')
				{
					ok = true;
					break;
				}
			if (!ok)
				break;
			std::cout << treeString[i] << '\n';
		}
		std::cout << '\n';
	}

};

int main()
{
	//read the name of the file and open stream
	std::string filename;
	std::cin >> filename;
	std::cout << '\n';

	std::ifstream fin(filename.c_str());

	if (fin.good())
	{
		char exp[512]; //buffer for expressions
		while (fin.getline(exp, 512))
		{
			std::cout << "Expression: " << exp << '\n';
			std::string expstring(exp);
			Tree t(expstring);

			t.print();
			t.display();
			std::cout << "\\______________________________________________________/\n\n";
		}
	}
	else
		std::cout << "Error! Unable to open file\n";

	getchar();
	getchar();
}
