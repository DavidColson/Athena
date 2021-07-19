#include "Json.h"

#include "Log.h"
#include "ErrorHandling.h"
#include "Scanning.h"

namespace An
{
	// Tokenizer
	///////////////////////////

	enum TokenType
	{
		// Single characters
		LeftBracket,
		RightBracket,
		LeftBrace,
		RightBrace,
		Comma,
		Colon,

		// Identifiers and keywords
		Boolean,
		Null,
		Identifier,
		
		// Everything else
		Number,
		String
	};

	struct Token
	{
		Token(TokenType type, int _line, int _column, int _index)
		: m_type(type), m_line(_line), m_column(_column), m_index(_index) {}

		Token(TokenType type, int _line, int _column, int _index, eastl::string _stringOrIdentifier)
		: m_type(type), m_line(_line), m_column(_column), m_index(_index), m_stringOrIdentifier(_stringOrIdentifier) {}

		Token(TokenType type, int _line, int _column, int _index, double _number)
		: m_type(type), m_line(_line), m_column(_column), m_index(_index), m_number(_number) {}

		Token(TokenType type, int _line, int _column, int _index, bool _boolean)
		: m_type(type), m_line(_line), m_column(_column), m_index(_index), m_boolean(_boolean) {}

		TokenType m_type;
		eastl::string m_stringOrIdentifier;
		double m_number;
		bool m_boolean;

		int m_line;
		int m_column;
		int m_index;
	};

	// ***********************************************************************

	eastl::string ParseString(Scan::ScanningState& scan, char bound)
	{	
		int start = scan.current;
		eastl::string result;
		while (Scan::Peek(scan) != bound && !Scan::IsAtEnd(scan))
		{
			char c = Advance(scan);
			
			// Disallowed characters
			switch (c)
			{
			case '\n':
				break;
			case '\r':
				if (Scan::Peek(scan) == '\n') // CRLF line endings
					Scan::Advance(scan);
				break;
				Scan::HandleError(scan, "Unexpected end of line", scan.current-1); break;
			default:
				break;
			}

			if (c == '\\')
			{
				char next = Advance(scan);
				switch (next)
				{
				// Convert basic escape sequences to their actual characters
				case '\'': result += '\''; break;
				case '"': result += '"'; break;
				case '\\':result += '\\'; break;
				case 'b': result += '\b'; break;
				case 'f': result += '\f'; break;
				case 'n': result += '\n'; break;
				case 'r': result += '\r'; break;
				case 't': result += '\t'; break;
				case 'v': result += '\v'; break;
				case '0':result += '\0'; break;

				// Unicode stuff, not doing this for now
				case 'u':
					Scan::HandleError(scan, "This parser does not yet support unicode escape codes", scan.current - 1); break;
				
				// Line terminators, allowed but we do not include them in the final string
				case '\n':
					break;
				case '\r':
					if (Scan::Peek(scan) == '\n') // CRLF line endings
						Scan::Advance(scan);
					break;
				default:
					result += next; // all other escaped characters are kept as is, without the '\' that preceeded it
				}
			}
			else
				result += c;
		}
		Scan::Advance(scan);
		return result;
	}

	// ***********************************************************************

	double ParseNumber(Scan::ScanningState& scan)
	{	
		scan.current -= 1; // Go back to get the first digit or symbol
		int start = scan.current;

		// Hex number
		if (Scan::Peek(scan) == '0' && (Scan::PeekNext(scan) == 'x' || Scan::PeekNext(scan) == 'X'))
		{
			Scan::Advance(scan);
			Scan::Advance(scan);
			while (Scan::IsHexDigit(Scan::Peek(scan)))
			{
				Scan::Advance(scan);
			}
		}
		// Normal number
		else
		{
			char c = Scan::Peek(scan);
			while(Scan::IsDigit(c) 
			|| c== '+'
			|| c == '-'
			|| c == '.'
			|| c == 'E'
			|| c == 'e')
			{
				Scan::Advance(scan);
				c = Scan::Peek(scan);
			}
		}

		// TODO: error report. This returns 0.0 if no conversion possible. We can look at the literal string and see
		// If it's 0.0, ".0", "0." or 0. if not there's been an error in the parsing. I know this is cheeky. I don't care.
		return strtod(scan.file.substr(start, (scan.current - start)).c_str(), nullptr);
	}

	// ***********************************************************************

	eastl::vector<Token> TokenizeJson(eastl::string jsonText)
	{
		Scan::ScanningState scan;
		scan.file = jsonText;
		scan.current = 0;
		scan.line = 1;

		eastl::vector<Token> tokens;

		while (!Scan::IsAtEnd(scan))
		{
			char c = Scan::Advance(scan);
			int column = scan.current - scan.currentLineStart;
			int loc = scan.current - 1;
			switch (c)
			{
			// Single character tokens
			case '[': 
				tokens.push_back(Token{LeftBracket, scan.line, column, loc}); break;
			case ']': 
				tokens.push_back(Token{RightBracket, scan.line, column, loc}); break;
			case '{': 
				tokens.push_back(Token{LeftBrace, scan.line, column,loc}); break;
			case '}': 
				tokens.push_back(Token{RightBrace, scan.line, column, loc}); break;
			case ':': 
				tokens.push_back(Token{Colon, scan.line, column, loc}); break;
			case ',': 
				tokens.push_back(Token{Comma, scan.line, column, loc}); break;

			// Comments!
			case '/':
				if (Scan::Match(scan, '/'))
				{
					while (Scan::Peek(scan) != '\n')
						Scan::Advance(scan);
				}
				else if (Scan::Match(scan, '*'))
				{
					while (!(Scan::Peek(scan) == '*' && Scan::PeekNext(scan) == '/'))
						Scan::Advance(scan);

					Scan::Advance(scan); // *
					Scan::Advance(scan); // /
				}
				break;
				
			// Whitespace
			case ' ':
			case '\r':
			case '\n':
			case '\t':
				break;

			// String literals
			case '\'':
			{
				eastl::string string = ParseString(scan, '\'');
				tokens.push_back(Token{String, scan.line, column, loc, string}); break;
				break;
			}
			case '"':
			{
				eastl::string string = ParseString(scan, '"');
				tokens.push_back(Token{String, scan.line, column, loc, string}); break;
				break;		
			}

			default:
				// Numbers
				if (Scan::IsDigit(c) || c == '+' || c == '-' || c == '.')
				{
					double num = ParseNumber(scan);
					tokens.push_back(Token{Number, scan.line, column, loc, num});
					break;
				}
				
				// Identifiers and keywords
				if (Scan::IsAlpha(c))
				{
					while (Scan::IsAlphaNumeric(Scan::Peek(scan)))
						Scan::Advance(scan);
					
					eastl::string identifier = scan.file.substr(loc, scan.current - loc);

					// Check for keywords
					if (identifier == "true")
						tokens.push_back(Token{Boolean, scan.line, column, loc, true});
					else if (identifier == "false")
						tokens.push_back(Token{Boolean, scan.line, column, loc, false});
					else if (identifier == "null")
						tokens.push_back(Token{Null, scan.line, column, loc});
					else
						tokens.push_back(Token{Identifier, scan.line, column, loc, identifier});
				}
				break;
			}

		}
		return tokens;
	}




	// ***********************************************************************

	eastl::map<eastl::string, JsonValue> ParseObject(eastl::vector<Token>& tokens, int& currentToken);
	eastl::vector<JsonValue> ParseArray(eastl::vector<Token>& tokens, int& currentToken);

	JsonValue ParseValue(eastl::vector<Token>& tokens, int& currentToken)
	{
		Token& token = tokens[currentToken];

		switch (token.m_type)
		{
		case LeftBrace:
			return JsonValue(ParseObject(tokens, currentToken)); break;
		case LeftBracket:
			return JsonValue(ParseArray(tokens, currentToken)); break;
		case String:
			currentToken++;
			return JsonValue(token.m_stringOrIdentifier); break;
		case Number:
		{
			currentToken++;
			double n = token.m_number;
			double intPart;
			if (modf(n, &intPart) == 0.0)
				return JsonValue((long)intPart);
			else
				return JsonValue(n);
			break;
		}
		case Boolean:
			currentToken++;
			return JsonValue(token.m_boolean); break;
		case Null:
			currentToken++;
		default:
			return JsonValue(); break;
		}
		return JsonValue();
	}

	// ***********************************************************************

	eastl::map<eastl::string, JsonValue> ParseObject(eastl::vector<Token>& tokens, int& currentToken)
	{
		currentToken++; // Advance over opening brace

		eastl::map<eastl::string, JsonValue> map;
		while (currentToken < tokens.size() && tokens[currentToken].m_type != RightBrace)
		{
			// We expect, 
			// identifier or string
			if (tokens[currentToken].m_type != Identifier && tokens[currentToken].m_type != String)
				Log::Crit("Expected identifier or string");

			eastl::string key = tokens[currentToken].m_stringOrIdentifier;
			currentToken += 1;

			// colon
			if (tokens[currentToken].m_type != Colon)
				Log::Crit("Expected colon");
			currentToken += 1;
			
			// String, Number, Boolean, Null
			// If left bracket or brace encountered, skip until closing
			map[key] = ParseValue(tokens, currentToken);

			// Comma, or right brace
			if (tokens[currentToken].m_type == RightBrace)
				break;
			if (tokens[currentToken].m_type != Comma)
				Log::Crit("Expected comma or Right Curly Brace");
			currentToken += 1;
		}
		currentToken++; // Advance over closing brace
		return map;
	}

	// ***********************************************************************

	eastl::vector<JsonValue> ParseArray(eastl::vector<Token>& tokens, int& currentToken)
	{
		currentToken++; // Advance over opening bracket

		eastl::vector<JsonValue> array;
		while (currentToken < tokens.size() && tokens[currentToken].m_type != RightBracket)
		{
			// We expect, 
			// String, Number, Boolean, Null
			array.push_back(ParseValue(tokens, currentToken));

			// Comma, or right brace
			if (tokens[currentToken].m_type == RightBracket)
				break;
			if (tokens[currentToken].m_type != Comma)
				Log::Crit("Expected comma or right bracket");
			currentToken += 1;
		}
		currentToken++; // Advance over closing bracket
		return array;
	}




	// JsonValue implementation
	///////////////////////////

	// ***********************************************************************

	JsonValue::~JsonValue()
	{
		if (m_type == Type::Array)
			delete m_internalData.m_pArray;	
		else if (m_type == Type::Object)
			delete m_internalData.m_pObject;
		else if (m_type == Type::String)
		{
			delete m_internalData.m_pString;
		}
		m_type = Type::Null;
	}

	// ***********************************************************************

	JsonValue::JsonValue()
	{
		m_type = Type::Null;
		m_internalData.m_pArray = nullptr;
	}

	// ***********************************************************************

	JsonValue::JsonValue(const JsonValue& copy)
	{
		// Copy data from the other value
		switch (copy.m_type)
		{
		case Type::Array:
			m_internalData.m_pArray = new eastl::vector<JsonValue>(*copy.m_internalData.m_pArray);
			break;
		case Type::Object:
			m_internalData.m_pObject = new eastl::map<eastl::string, JsonValue>(*copy.m_internalData.m_pObject);
			break;
		case Type::String:
			m_internalData.m_pString = new eastl::string(*(copy.m_internalData.m_pString));
			break;
		default:
			m_internalData = copy.m_internalData;
			break;
		}
		m_type = copy.m_type;
	}

	// ***********************************************************************

	JsonValue::JsonValue(JsonValue&& copy)
	{
		m_internalData = copy.m_internalData;
		copy.m_internalData.m_pArray = nullptr;

		m_type = copy.m_type;
		copy.m_type = Type::Null;	
	}

	// ***********************************************************************

	JsonValue& JsonValue::operator=(const JsonValue& copy)
	{
		// Clear out internal stuff
		if (m_type == Type::Array)
			delete m_internalData.m_pArray;	
		else if (m_type == Type::Object)
			delete m_internalData.m_pObject;
		else if (m_type == Type::String)
			delete m_internalData.m_pString;

		// Copy data from the other value
		switch (copy.m_type)
		{
		case Type::Array:
			m_internalData.m_pArray = new eastl::vector<JsonValue>(copy.m_internalData.m_pArray->begin(), copy.m_internalData.m_pArray->end());
			break;
		case Type::Object:
			m_internalData.m_pObject = new eastl::map<eastl::string, JsonValue>(copy.m_internalData.m_pObject->begin(), copy.m_internalData.m_pObject->end());
			break;
		case Type::String:
			m_internalData.m_pString = new eastl::string(*(copy.m_internalData.m_pString));
		default:
			m_internalData = copy.m_internalData;
			break;
		}
		m_type = copy.m_type;

		return *this;
	}

	// ***********************************************************************

	JsonValue& JsonValue::operator=(JsonValue&& copy)
	{
		m_internalData = copy.m_internalData;
		copy.m_internalData.m_pArray = nullptr;

		m_type = copy.m_type;
		copy.m_type = Type::Null;	

		return *this;
	}

	// ***********************************************************************

	JsonValue::JsonValue(eastl::vector<JsonValue>& array)
	{
		m_internalData.m_pArray = nullptr;
		m_internalData.m_pArray = new eastl::vector<JsonValue>(array.begin(), array.end());
		m_type = Type::Array;
	}

	// ***********************************************************************

	JsonValue::JsonValue(eastl::map<eastl::string, JsonValue>& object)
	{
		m_internalData.m_pArray = nullptr;
		m_internalData.m_pObject = new eastl::map<eastl::string, JsonValue>(object.begin(), object.end());
		m_type = Type::Object;
	}

	// ***********************************************************************

	JsonValue::JsonValue(eastl::string string)
	{
		m_internalData.m_pArray = nullptr;
		m_internalData.m_pString = new eastl::string(string);
		m_type = Type::String;
	}

	// ***********************************************************************

	JsonValue::JsonValue(const char* string)
	{
		m_internalData.m_pArray = nullptr;
		m_internalData.m_pString = new eastl::string(string);
		m_type = Type::String;
	}

	// ***********************************************************************

	JsonValue::JsonValue(double number)
	{
		m_internalData.m_pArray = nullptr;
		m_internalData.m_floatingNumber = number;
		m_type = Type::Floating;
	}

	// ***********************************************************************

	JsonValue::JsonValue(long number)
	{
		m_internalData.m_pArray = nullptr;
		m_internalData.m_integerNumber = number;
		m_type = Type::Integer;
	}

	// ***********************************************************************

	JsonValue::JsonValue(bool boolean)
	{
		m_internalData.m_pArray = nullptr;
		m_internalData.m_boolean = boolean;
		m_type = Type::Boolean;
	}

	// ***********************************************************************

	eastl::string JsonValue::ToString() const
	{
		if (m_type == Type::String)
			return *(m_internalData.m_pString);
		return eastl::string();
	}

	// ***********************************************************************

	double JsonValue::ToFloat() const
	{
		if (m_type == Type::Floating)
			return m_internalData.m_floatingNumber;
		else if (m_type == Type::Integer)
			return (double)m_internalData.m_integerNumber;
		return 0.0f;
	}

	// ***********************************************************************

	long JsonValue::ToInt() const
	{
		if (m_type == Type::Integer)
			return m_internalData.m_integerNumber;
		return 0;
	}

	// ***********************************************************************

	bool JsonValue::ToBool() const
	{
		if (m_type == Type::Boolean)
			return m_internalData.m_boolean;
		return 0;
	}

	// ***********************************************************************

	bool JsonValue::IsNull() const
	{
		return m_type == Type::Null;
	}

	// ***********************************************************************

	bool JsonValue::HasKey(eastl::string identifier) const
	{
		return m_internalData.m_pObject->count(identifier) >= 1;
	}

	// ***********************************************************************

	int JsonValue::Count() const
	{
		ASSERT(m_type == Type::Array || m_type == Type::Object, "Attempting to treat this value as an array or object when it is not.");
		if (m_type == Type::Array)
			return (int)m_internalData.m_pArray->size();
		else
			return (int)m_internalData.m_pObject->size();
	}

	// ***********************************************************************

	JsonValue& JsonValue::operator[](eastl::string identifier)
	{
		ASSERT(m_type == Type::Object, "Attempting to treat this value as an object when it is not.");
		return m_internalData.m_pObject->operator[](identifier);
	}

	// ***********************************************************************

	JsonValue& JsonValue::operator[](size_t index)
	{
		ASSERT(m_type == Type::Array, "Attempting to treat this value as an array when it is not.");
		ASSERT(m_internalData.m_pArray->size() > index, "Accessing an element that does not exist in this array, you probably need to append");
		return m_internalData.m_pArray->operator[](index);
	}

	// ***********************************************************************

	const JsonValue& JsonValue::Get(eastl::string identifier) const
	{
		ASSERT(m_type == Type::Object, "Attempting to treat this value as an object when it is not.");
		return m_internalData.m_pObject->operator[](identifier);
	}

	// ***********************************************************************

	const JsonValue& JsonValue::Get(size_t index) const
	{
		ASSERT(m_type == Type::Array, "Attempting to treat this value as an array when it is not.");
		ASSERT(m_internalData.m_pArray->size() > index, "Accessing an element that does not exist in this array, you probably need to append");
		return m_internalData.m_pArray->operator[](index);
	}

	// ***********************************************************************

	void JsonValue::Append(JsonValue& value)
	{
		ASSERT(m_type == Type::Array, "Attempting to treat this value as an array when it is not.");
		m_internalData.m_pArray->push_back(value);
	}

	// ***********************************************************************

	JsonValue JsonValue::NewObject()
	{
		return JsonValue(eastl::map<eastl::string, JsonValue>());
	}

	// ***********************************************************************

	JsonValue JsonValue::NewArray()
	{
		return JsonValue(eastl::vector<JsonValue>());
	}

	// ***********************************************************************

	JsonValue ParseJsonFile(eastl::string& file)
	{
		eastl::vector<Token> tokens = TokenizeJson(file);

		int firstToken = 0;
		JsonValue json5 = ParseValue(tokens, firstToken);
		return json5;
	}

	// ***********************************************************************

	eastl::string SerializeJsonValue(JsonValue json, eastl::string indentation)
	{
		eastl::string result = "";
		switch (json.m_type)
		{
		case JsonValue::Type::Array:
			result.append("[");
			if (json.Count() > 0)
				result.append("\n");

			for (const JsonValue& val : *json.m_internalData.m_pArray)
			{
				result.append_sprintf("    %s%s, \n", indentation.c_str(), SerializeJsonValue(val, indentation + "    ").c_str());
			}

			if (json.Count() > 0)
				result.append_sprintf("%s", indentation.c_str());
			result.append("]");
			break;
		case JsonValue::Type::Object:
		{
			result.append("{");
			if (json.Count() > 0)
				result.append("\n");

			for (const eastl::pair<eastl::string, JsonValue>& val : *json.m_internalData.m_pObject)
			{
				result.append_sprintf("    %s%s: %s, \n", indentation.c_str(), val.first.c_str(), SerializeJsonValue(val.second, indentation + "    ").c_str());
			}

			if (json.Count() > 0)
				result.append_sprintf("%s", indentation.c_str());
			result.append("}");
		}
		break;
		case JsonValue::Type::Floating:
			result.append_sprintf("%.17g", json.ToFloat());
			break;
			// TODO: Serialize with exponentials like we do with floats
		case JsonValue::Type::Integer:
			result.append_sprintf("%i", json.ToInt());
			break;
		case JsonValue::Type::Boolean:
			result.append_sprintf("%s", json.ToBool() ? "true" : "false");
			break;
		case JsonValue::Type::String:
			result.append_sprintf("\"%s\"", json.ToString().c_str());
			break;
		case JsonValue::Type::Null:
			result.append("null");
			break;
		default:
			result.append("CANT SERIALIZE YET");
			break;
		}

		// Kinda hacky thing that makes small jsonValues serialize as collapsed, easier to read files imo
		if (result.size() < 100)
		{
			size_t index = 0;
			while (true) {
				index = result.find("\n", index);

				int count = 1;
				while((index+count) != eastl::string::npos && result[index+count] == ' ')
					count++;

				if (index == eastl::string::npos) break;
				result.replace(index, count, "");
				index += 1;
			}
		}
		return result;
	}
}