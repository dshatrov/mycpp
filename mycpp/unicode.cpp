#include <mycpp/unicode.h>

namespace MyCpp {

// Unicode code points for newline characters.
class NewlineCharacter
{
public:
    enum _NewlineCharacter {
	CR  = 0x0d,
	LF  = 0x0a,
	NEL = 0x85,
	VT  = 0x0b, // not a newline character
	FF  = 0x0c,
	LS  = 0x2028,
	PS  = 0x2029
    };

    _NewlineCharacter value;

    operator _NewlineCharacter () const
    {
	return value;
    }

    NewlineCharacter (_NewlineCharacter const &value)
    {
	this->value = value;
    }
};

bool unicode_isAlphanumeric (Unichar c)
{
    return g_unichar_isalnum (c);
}

bool unicode_isAlpha (Unichar c)
{
    return g_unichar_isalpha (c);
}

bool unicode_isDigit (Unichar c)
{
    return g_unichar_isdigit (c);
}

bool unicode_isSpace (Unichar c)
{
    return g_unichar_isspace (c);
}

FuzzyResult unicode_isNewline (Unichar c)
{
    switch (c) {
	case NewlineCharacter::CR:
	    return FuzzyResult::Maybe;
	case NewlineCharacter::LF:
	case NewlineCharacter::NEL:
	case NewlineCharacter::FF:
	case NewlineCharacter::LS:
	case NewlineCharacter::PS:
	    return FuzzyResult::Yes;
	default:
	    return FuzzyResult::No;
    }
}

FuzzyResult unicode_isNewline (Iterator<Unichar const &> &char_iter,
			       Size *ret_num_chars)
{
    if (ret_num_chars != NULL)
	*ret_num_chars = 0;

    Bool cr;
    while (!char_iter.done ())
    {
	Unichar const &c = char_iter.next ();

	if (cr) {
	    if (ret_num_chars != NULL) {
		if (c == NewlineCharacter::LF)
		    *ret_num_chars = 2;
		else
		    *ret_num_chars = 1;
	    }

	    return FuzzyResult::Yes;
	}

	switch (c) {
	    case NewlineCharacter::CR:
		cr = true;
		break;
	    case NewlineCharacter::LF:
	    case NewlineCharacter::NEL:
	    case NewlineCharacter::FF:
	    case NewlineCharacter::LS:
	    case NewlineCharacter::PS:
		*ret_num_chars = 1;
		return FuzzyResult::Yes;
	    default:
		return FuzzyResult::No;
	}

    }

    abortIf (!cr);
    if (ret_num_chars != NULL)
	*ret_num_chars = 1;
    return FuzzyResult::Maybe;
}

}

