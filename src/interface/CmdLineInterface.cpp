#include "lib/config.h"

#if defined(HAVE_CMDLINE)

#include "interface/CmdLineInterface.h"
#include "interface/SGInterface.h"

#include "lib/ShogunException.h"
#include "lib/io.h"
#include "lib/SimpleFile.h"

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#ifndef WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

const INT READLINE_BUFFER_SIZE = 10000;
extern CSGInterface* interface;
extern CSGInterfaceMethod sg_methods[];

CCmdLineInterface::CCmdLineInterface()
: CSGInterface(), m_lhs(NULL), m_rhs(NULL)
{
	reset();
}

CCmdLineInterface::~CCmdLineInterface()
{
	delete m_rhs;
}

void CCmdLineInterface::reset(const CHAR* line)
{
	CSGInterface::reset();

	if (!line)
		return;

	CHAR* element=NULL;
	CHAR delim_equal[]="=";
	CHAR delim_lhs[]="=, \t\n";
	CHAR delim_rhs[]=" \t\n";

	delete m_lhs;
	m_lhs=NULL;
	delete m_rhs;
	m_rhs=NULL;

	// split lhs from rhs
	CHAR* equal_sign=strstr(line, delim_equal);
	if (equal_sign)
	//if (strstr(line, delim_equal))
	{
#ifdef DEBUG_CMDLINEIF
		SG_PRINT("has lhs!\n");
#endif
		element=strtok((CHAR*) line, delim_lhs);
		if (element)
		{
			m_lhs=new CDynamicArray<CHAR*>();
			m_lhs->append_element(element);
			m_nlhs++;
			while ((element=strtok(NULL, delim_lhs)))
			{
				if (element>equal_sign) // on rhs now
					break;

				m_lhs->append_element(element);
				m_nlhs++;
			}
		}
	}
	else
		element=strtok((CHAR*) line, delim_rhs);

	if (element)
	{
		m_rhs=new CDynamicArray<CHAR*>();
		m_rhs->append_element(element);
		m_nrhs++;
		while ((element=strtok(NULL, delim_rhs)))
		{
			m_rhs->append_element(element);
			m_nrhs++;
		}
	}
	else
		m_rhs=NULL;

#ifdef DEBUG_CMDLINEIF
	SG_PRINT("nlhs=%d nrhs=%d\n", m_nlhs, m_nrhs);
	if (m_lhs)
	{
		for (INT i=0; i<m_lhs->get_num_elements(); i++)
			SG_PRINT("element lhs %i %s\n", i, m_lhs->get_element(i));
	}

	if (m_rhs)
	{
		for (INT i=0; i<m_rhs->get_num_elements(); i++)
			SG_PRINT("element rhs %i %s\n", i, m_rhs->get_element(i));
	}
#endif
}


/** get functions - to pass data from the target interface to shogun */

/** determine argument type
 *
 * a signature is read from a data file. this signature contains the argument
 * type.
 *
 * currently, the signature must be in the beginning of the file,
 * consists of a line starting with 3 hash signs, 1 space and then
 * the argument type. e.g.:
 *
 * ### SHOGUN V0 STRING_CHAR
 * ACGTGCAAAAGC
 * AGTCDTCD
 */
IFType CCmdLineInterface::get_argument_type()
{
	const INT len=1024;
	IFType argtype=UNDEFINED;
	const CHAR* filename=m_rhs->get_element(m_rhs_counter);

	// read the first 1024 of the file and heuristically decide about its
	// content
	FILE* fh=fopen((CHAR*) filename, "r");
	if (!fh)
		SG_ERROR("Could not find file %s.\n", filename);

	CHAR* chunk=new CHAR[len+1];
	ASSERT(chunk);
	memset(chunk, 0, sizeof(CHAR)*(len+1));
	size_t nread=fread(chunk, sizeof(CHAR), len, fh);
	fclose(fh);
	if (nread<=0)
		SG_ERROR("Could not read data from %s.\n");

	CHAR* signature=new CHAR[len+1];
	ASSERT(signature);
	INT num=sscanf(chunk, "### SHOGUN V0 %s\n", signature);

	// if file has valid shogun signature use it to determine file type
	if (num==1)
	{
		SG_DEBUG("read signature: %s\n", signature);

		if (strncmp(signature, "STRING_CHAR", 11)==0)
			argtype=STRING_CHAR;
		else if (strncmp(signature, "STRING_BYTE", 11)==0)
			argtype=STRING_BYTE;
		else if (strncmp(signature, "DENSE_REAL", 10)==0)
			argtype=DENSE_REAL;
		else if (strncmp(signature, "SPARSE_REAL", 11)==0)
			argtype=SPARSE_REAL;
	}
	else
	{
		SG_DEBUG("could not find signature in file %s guessing file type.\n", filename);
		
		if (strspn(chunk, "0123456789.e+- \t\n")==nread)
		{
			argtype=DENSE_REAL;
			SG_DEBUG("guessing DENSE_REAL\n");
		}
		else if (strspn(chunk, "0123456789:.e+- \t\n")==nread)
		{
			argtype=SPARSE_REAL;
			SG_DEBUG("guessing SPARSE_REAL\n");
		}
		else
		{
			argtype=STRING_CHAR;
			SG_DEBUG("guessing STRING_CHAR\n");
		}
	}

	delete[] signature;
	delete[] chunk;
	return argtype;
}


INT CCmdLineInterface::get_int()
{
	const CHAR* i=get_arg_increment();
	if (!i)
		SG_ERROR("Expected Scalar Integer as argument %d\n", m_rhs_counter);

	INT value=-1;
	INT num=sscanf(i, "%d", &value);
	if (num!=1)
		SG_ERROR("Expected Scalar Integer as argument %d\n", m_rhs_counter);

	return value;
}

DREAL CCmdLineInterface::get_real()
{
	const CHAR* r=get_arg_increment();
	if (!r)
		SG_ERROR("Expected Scalar Real as argument %d\n", m_rhs_counter);

	DREAL value=-1;
	INT num=sscanf(r, "%lf", &value);
	if (num!=1)
		SG_ERROR("Expected Scalar Real as argument %d\n", m_rhs_counter);

	return value;
}

bool CCmdLineInterface::get_bool()
{
	const CHAR* b=get_arg_increment();
	if (!b)
		SG_ERROR("Expected Scalar Bool as argument %d\n", m_rhs_counter);

	INT value=-1;
	INT num=sscanf(b, "%i", &value);
	if (num!=1)
		SG_ERROR("Expected Scalar Bool as argument %d\n", m_rhs_counter);

	return (value!=0);
}


CHAR* CCmdLineInterface::get_string(INT& len)
{
	const CHAR* s=get_arg_increment();
	if (!s)
		SG_ERROR("Expected 1 String as argument %d.\n", m_rhs_counter);

	len=strlen(s);
	ASSERT(len>0);

	CHAR* result=new CHAR[len+1];
	memcpy(result, s, len*sizeof(CHAR));
	result[len]='\0';

	return result;
}

void CCmdLineInterface::get_byte_vector(BYTE*& vec, INT& len)
{
	vec=NULL;
	len=0;
}

void CCmdLineInterface::get_char_vector(CHAR*& vec, INT& len)
{
	vec=NULL;
	len=0;
}

void CCmdLineInterface::get_int_vector(INT*& vec, INT& len)
{
	vec=NULL;
	len=0;
/*
	vec=NULL;
	len=0;

	void* rvec=CAR(get_arg_increment());
	if( TYPEOF(rvec) != INTSXP )
		SG_ERROR("Expected Integer Vector as argument %d\n", m_rhs_counter);

	len=LENGTH(rvec);
	vec=new INT[len];
	ASSERT(vec);

	for (INT i=0; i<len; i++)
		vec[i]= (INT) INTEGER(rvec)[i];
		*/
}

void CCmdLineInterface::get_shortreal_vector(SHORTREAL*& vec, INT& len)
{
	vec=NULL;
	len=0;
}

void CCmdLineInterface::get_real_vector(DREAL*& vec, INT& len)
{
	vec=NULL;
	len=0;

	const CHAR* filename=get_arg_increment();
	if (!filename)
		SG_ERROR("No filename given to read REAL matrix.\n");

	CFile f((CHAR*) filename, 'r', F_DREAL);
	if (!f.is_ok())
		SG_ERROR("Could not open file %s to read REAL matrix.\n", filename);

	INT num_feat=0;
	INT num_vec=0;

	if (!f.read_real_valued_dense(vec, num_feat, num_vec))
		SG_ERROR("Could not read REAL data from %s.\n", filename);

	if ((num_feat==1) || (num_vec==1))
	{
		if (num_feat==1)
			len=num_vec;
		else
			len=num_feat;
	}
	else
	{
		delete[] vec;
		vec=NULL;
		len=0;
		SG_ERROR("Could not read REAL vector from file %s (shape %dx%d found but vector expected).\n", filename, num_vec, num_feat);
	}

}

void CCmdLineInterface::get_short_vector(SHORT*& vec, INT& len)
{
	vec=NULL;
	len=0;
}

void CCmdLineInterface::get_word_vector(WORD*& vec, INT& len)
{
	vec=NULL;
	len=0;
}


void CCmdLineInterface::get_byte_matrix(BYTE*& matrix, INT& num_feat, INT& num_vec)
{
	matrix=NULL;
	num_feat=0;
	num_vec=0;
}

void CCmdLineInterface::get_char_matrix(CHAR*& matrix, INT& num_feat, INT& num_vec)
{
	matrix=NULL;
	num_feat=0;
	num_vec=0;
}

void CCmdLineInterface::get_int_matrix(INT*& matrix, INT& num_feat, INT& num_vec)
{
	matrix=NULL;
	num_feat=0;
	num_vec=0;
}

void CCmdLineInterface::get_shortreal_matrix(SHORTREAL*& matrix, INT& num_feat, INT& num_vec)
{
	matrix=NULL;
	num_feat=0;
	num_vec=0;
}

void CCmdLineInterface::get_real_matrix(DREAL*& matrix, INT& num_feat, INT& num_vec)
{
	const CHAR* filename=get_arg_increment();
	if (!filename)
		SG_ERROR("No filename given to read REAL matrix.\n");

	CFile f((CHAR*) filename, 'r', F_DREAL);
	if (!f.is_ok())
		SG_ERROR("Could not open file %s to read REAL matrix.\n", filename);

	if (!f.read_real_valued_dense(matrix, num_feat, num_vec))
		SG_ERROR("Could not read REAL data from %s.\n", filename);
}

void CCmdLineInterface::get_short_matrix(SHORT*& matrix, INT& num_feat, INT& num_vec)
{
	matrix=NULL;
	num_feat=0;
	num_vec=0;
}

void CCmdLineInterface::get_word_matrix(WORD*& matrix, INT& num_feat, INT& num_vec)
{
	matrix=NULL;
	num_feat=0;
	num_vec=0;
}


void CCmdLineInterface::get_real_sparsematrix(TSparse<DREAL>*& matrix, INT& num_feat, INT& num_vec)
{
	const CHAR* filename=get_arg_increment();
	if (!filename)
		SG_ERROR("No filename given to read SPARSE REAL matrix.\n");

	CFile f((CHAR*) filename, 'r', F_DREAL);
	if (!f.is_ok())
		SG_ERROR("Could not open file %s to read SPARSE REAL matrix.\n", filename);

	if (!f.read_real_valued_sparse(matrix, num_feat, num_vec))
		SG_ERROR("Could not read SPARSE REAL data from %s.\n", filename);
}


void CCmdLineInterface::get_byte_string_list(T_STRING<BYTE>*& strings, INT& num_str, INT& max_string_len)
{
	strings=NULL;
	num_str=0;
	max_string_len=0;
}

void CCmdLineInterface::get_char_string_list(T_STRING<CHAR>*& strings, INT& num_str, INT& max_string_len)
{
	const CHAR* filename=get_arg_increment();
	if (!filename)
		SG_ERROR("No filename given to read CHAR string list.\n");

	CFile f((CHAR*) filename, 'r', F_CHAR);
	if (!f.is_ok())
		SG_ERROR("Could not open file %s to read CHAR string list.\n", filename);

	if (!f.read_char_valued_strings(strings, num_str, max_string_len))
		SG_ERROR("Could not read CHAR data from %s.\n", filename);

/*
	for (INT i=0; i<num_str; i++)
		SG_PRINT("%s\n", strings[i].string);
*/
}

void CCmdLineInterface::get_int_string_list(T_STRING<INT>*& strings, INT& num_str, INT& max_string_len)
{
	strings=NULL;
	num_str=0;
	max_string_len=0;
}

void CCmdLineInterface::get_short_string_list(T_STRING<SHORT>*& strings, INT& num_str, INT& max_string_len)
{
	strings=NULL;
	num_str=0;
	max_string_len=0;
}

void CCmdLineInterface::get_word_string_list(T_STRING<WORD>*& strings, INT& num_str, INT& max_string_len)
{
	strings=NULL;
	num_str=0;
	max_string_len=0;
}

/** set functions - to pass data from shogun to the target interface */
bool CCmdLineInterface::create_return_values(INT num)
{
	if (num==m_nlhs)
		return true;

	return false;
}

void* CCmdLineInterface::get_return_values()
{
	return NULL;
}


/** set functions - to pass data from shogun to the target interface */

void CCmdLineInterface::set_int(INT scalar)
{
	//set_arg_increment(ScalarInteger(scalar));
}

void CCmdLineInterface::set_real(DREAL scalar)
{
	//set_arg_increment(ScalarReal(scalar));
}

void CCmdLineInterface::set_bool(bool scalar)
{
	//set_arg_increment(ScalarLogical(scalar));
}


void CCmdLineInterface::set_char_vector(const CHAR* vec, INT len)
{
}

void CCmdLineInterface::set_short_vector(const SHORT* vec, INT len)
{
}

void CCmdLineInterface::set_byte_vector(const BYTE* vec, INT len)
{
}

void CCmdLineInterface::set_int_vector(const INT* vec, INT len)
{
}

void CCmdLineInterface::set_shortreal_vector(const SHORTREAL* vec, INT len)
{
}

void CCmdLineInterface::set_real_vector(const DREAL* vec, INT len)
{
	const CHAR* filename=set_arg_increment();
	if (!filename)
		SG_ERROR("No filename given to write REAL vector.\n");

	CFile f((CHAR*) filename, 'w', F_DREAL);
	if (!f.is_ok())
		SG_ERROR("Could not open file %s to write REAL vector.\n", filename);

	if (!f.write_real_valued_dense(vec, len, 1))
		SG_ERROR("Could not write REAL data to %s.\n", filename);
}

void CCmdLineInterface::set_word_vector(const WORD* vec, INT len)
{
}

/*
#undef SET_VECTOR
#define SET_VECTOR(function_name, r_type, r_cast, sg_type, if_type, error_string) \
void CCmdLineInterface::function_name(const sg_type* vec, INT len)	\
{																\
	void* feat=NULL;												\
	PROTECT( feat = allocVector(r_type, len) );					\
																\
	for (INT i=0; i<len; i++)									\
		r_cast(feat)[i]=(if_type) vec[i];						\
																\
	UNPROTECT(1);												\
	set_arg_increment(feat);									\
}

SET_VECTOR(set_byte_vector, INTSXP, INTEGER, BYTE, int, "Byte")
SET_VECTOR(set_int_vector, INTSXP, INTEGER, INT, int, "Integer")
SET_VECTOR(set_short_vector, INTSXP, INTEGER, SHORT, int, "Short")
SET_VECTOR(set_shortreal_vector, XP, REAL, SHORTREAL, float, "Single Precision")
SET_VECTOR(set_real_vector, XP, REAL, DREAL, double, "Double Precision")
SET_VECTOR(set_word_vector, INTSXP, INTEGER, WORD, int, "Word")
#undef SET_VECTOR
*/


void CCmdLineInterface::set_char_matrix(const CHAR* matrix, INT num_feat, INT num_vec)
{
}
void CCmdLineInterface::set_byte_matrix(const BYTE* matrix, INT num_feat, INT num_vec)
{
}
void CCmdLineInterface::set_int_matrix(const INT* matrix, INT num_feat, INT num_vec)
{
}
void CCmdLineInterface::set_short_matrix(const SHORT* matrix, INT num_feat, INT num_vec)
{
}
void CCmdLineInterface::set_shortreal_matrix(const SHORTREAL* matrix, INT num_feat, INT num_vec)
{
}
void CCmdLineInterface::set_real_matrix(const DREAL* matrix, INT num_feat, INT num_vec)
{
	const CHAR* filename=set_arg_increment();
	if (!filename)
		SG_ERROR("No filename given to write REAL matrix.\n");

	CFile f((CHAR*) filename, 'w', F_DREAL);
	if (!f.is_ok())
		SG_ERROR("Could not open file %s to write REAL matrix.\n", filename);

	if (!f.write_real_valued_dense(matrix, num_feat, num_vec))
		SG_ERROR("Could not write REAL data to %s.\n", filename);
}
void CCmdLineInterface::set_word_matrix(const WORD* matrix, INT num_feat, INT num_vec)
{
}

/*
#define SET_MATRIX(function_name, r_type, r_cast, sg_type, if_type, error_string) \
void CCmdLineInterface::function_name(const sg_type* matrix, INT num_feat, INT num_vec) \
{																			\
	void* feat=NULL;															\
	PROTECT( feat = allocMatrix(r_type, num_feat, num_vec) );				\
																			\
	for (INT i=0; i<num_vec; i++)											\
	{																		\
		for (INT j=0; j<num_feat; j++)										\
			r_cast(feat)[i*num_feat+j]=(if_type) matrix[i*num_feat+j];		\
	}																		\
																			\
	UNPROTECT(1);															\
	set_arg_increment(feat);												\
}
SET_MATRIX(set_byte_matrix, INTSXP, INTEGER, BYTE, int, "Byte")
SET_MATRIX(set_int_matrix, INTSXP, INTEGER, INT, int, "Integer")
SET_MATRIX(set_short_matrix, INTSXP, INTEGER, SHORT, int, "Short")
SET_MATRIX(set_shortreal_matrix, XP, REAL, SHORTREAL, float, "Single Precision")
SET_MATRIX(set_real_matrix, XP, REAL, DREAL, double, "Double Precision")
SET_MATRIX(set_word_matrix, INTSXP, INTEGER, WORD, int, "Word")
#undef SET_MATRIX
*/


void CCmdLineInterface::set_real_sparsematrix(const TSparse<DREAL>* matrix, INT num_feat, INT num_vec, LONG nnz)
{
	const CHAR* filename=set_arg_increment();
	if (!filename)
		SG_ERROR("No filename given to write SPARSE REAL matrix.\n");

	CFile f((CHAR*) filename, 'w', F_DREAL);
	if (!f.is_ok())
		SG_ERROR("Could not open file %s to write SPARSE REAL matrix.\n", filename);

	if (!f.write_real_valued_sparse(matrix, num_feat, num_vec))
		SG_ERROR("Could not write SPARSE REAL data to %s.\n", filename);
}

void CCmdLineInterface::set_byte_string_list(const T_STRING<BYTE>* strings, INT num_str)
{
}

void CCmdLineInterface::set_char_string_list(const T_STRING<CHAR>* strings, INT num_str)
{
	const CHAR* filename=set_arg_increment();
	if (!filename)
		SG_ERROR("No filename given to write CHAR string list.\n");

	CFile f((CHAR*) filename, 'w', F_CHAR);
	if (!f.is_ok())
		SG_ERROR("Could not open file %s to write CHAR string list.\n", filename);

	if (!f.write_char_valued_strings(strings, num_str))
		SG_ERROR("Could not write CHAR data to %s.\n", filename);
}

void CCmdLineInterface::set_int_string_list(const T_STRING<INT>* strings, INT num_str)
{
}

void CCmdLineInterface::set_short_string_list(const T_STRING<SHORT>* strings, INT num_str)
{
}

void CCmdLineInterface::set_word_string_list(const T_STRING<WORD>* strings, INT num_str)
{
}


bool CCmdLineInterface::skip_line(const CHAR* line)
{
	if (!line)
		return true;

	if (line[0]=='\n' ||
		(line[0]=='\r' && line[1]=='\n') ||
		int(line[0])==0)
	{
		return true;
	}

	//SG_PRINT("ascii(0) %d, %c\n", int(line[0]), line[0]);

	CHAR* skipped=CIO::skip_blanks((CHAR*) line);
	if (skipped[0]=='#' || skipped[0]=='%')
		return true;

	return false;
}

void CCmdLineInterface::print_prompt()
{
	SG_PRINT( "\033[1;34mshogun\033[0m >> ");
	//SG_PRINT("shogun >> ");
}


CHAR* CCmdLineInterface::get_line(FILE* infile, bool interactive_mode)
{
	char* in=NULL;
	memset(input, 0, sizeof(input));

	if (feof(infile))
		return NULL;

#ifdef HAVE_READLINE
	if (interactive_mode)
	{
		in=readline("\033[1;34mshogun\033[0m >> ");
		if (in)
		{
			strncpy(input, in, sizeof(input));
			add_history(in);
			free(in);
		}
	}
	else
	{
		if (fgets(input, sizeof(input), infile)==NULL)
			return NULL;
		in=input;
	}
#else
	if (interactive_mode)
		print_prompt();
	if (fgets(input, sizeof(input), infile)==NULL)
		return NULL;
	in=input;
#endif

	if (in==NULL)
		return NULL;
	else
		return input;
}

bool CCmdLineInterface::parse_line(CHAR* line)
{
	if (!line)
		return false;
	
	if (skip_line(line))
		return true;
	else
	{
		((CCmdLineInterface*) interface)->reset(line);
		return interface->handle();
	}
}

#ifdef HAVE_READLINE
char* command_generator(const char *text, int state)
{
	static int list_index, len;
	char *name;

	/* If this is a new word to complete, initialize now.  This
	 *      includes saving the length of TEXT for efficiency, and
	 *           initializing the index variable to 0. */
	if (!state)
	{
		list_index = 0;
		len = strlen (text);
	}

	/* Return the next name which partially matches from the
	 *      command list. */
	while ((name = sg_methods[list_index].command))
	{
		list_index++;

		if (strncmp (name, text, len) == 0)
			return (strdup(name));
	}

	/* If no names matched, then return NULL. */
	return NULL;
}

/* Attempt to complete on the contents of TEXT.  START and END
 * bound the region of rl_line_buffer that contains the word to
 * complete.  TEXT is the word to complete.  We can use the entire
 * contents of rl_line_buffer in case we want to do some simple
 * parsing.  Returnthe array of matches, or NULL if there aren't
 * any. */
char** shogun_completion (const char *text, int start, int end)
{
	char **matches;

	matches = (char **)NULL;

	/* If this word is at the start of the line, then it is a command
	 *      to complete.  Otherwise it is the name of a file in the
	 *      current
	 *           directory. */
	if (start == 0)
		matches = rl_completion_matches (text, command_generator);

	return (matches);
}
#endif //HAVE_READLINE

int main(int argc, char* argv[])
{	
#ifdef HAVE_READLINE
	rl_readline_name = "shogun";
	rl_attempted_completion_function = shogun_completion;
#endif //HAVE_READLINE

	interface=new CCmdLineInterface();

	CCmdLineInterface* intf=(CCmdLineInterface*) interface;

	// interactive
	if (argc<=1)
	{
		while (true)
		{
			CHAR* l=intf->get_line();

			if (!l)
				break;

			try
			{
				intf->parse_line(l);
			}
			catch (ShogunException e) { }
		}
		delete interface;
		return 0;
	}

	// help
	if ( argc>2 || ((argc==2) && 
				( !strcmp(argv[1], "-h") || !strcmp(argv[1], "/?") || !strcmp(argv[1], "--help")) )
	   )
	{
		SG_SPRINT("\n\n");
		SG_SPRINT("usage: shogun [ -h | --help | /? | -i | filename ]\n\n");
		SG_SPRINT("if no options are given shogun enters interactive mode\n");
		SG_SPRINT("if filename is specified the commands will be read and executed from file\n");
		SG_SPRINT("if -i is specified shogun will listen on port 7367 from file\n");
		SG_SPRINT("==hex(sg), *dangerous* as commands from any source are accepted\n\n");
		delete interface;
		return 1;
	}

#ifndef CYGWIN
	// from tcp
	if ( argc==2 && !strcmp(argv[1], "-i"))
	{
		int s=socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in sa;
		sa.sin_family=AF_INET;
		sa.sin_port=htons(7367);
		sa.sin_addr.s_addr=INADDR_ANY;
		bzero(&(sa.sin_zero), 8);

		bind(s, (sockaddr*) (&sa), sizeof(sockaddr_in));
		listen(s, 1);
		int s2=accept(s, NULL, NULL);
		SG_SINFO( "accepting connection\n");

		CHAR input[READLINE_BUFFER_SIZE];
		do
		{
			bzero(input, sizeof(input));
			int length=read(s2, input, sizeof(input));
			if (length>0 && length<(int) sizeof(input))
				input[length]='\0';
			else
			{
				SG_SERROR( "error reading cmdline\n");
				return 1;
			}
		}
		while (intf->parse_line(input));
		delete interface;
		return 0;
	}
#endif

	// from file
	if (argc==2)
	{
		FILE* file=fopen(argv[1], "r");

		if (!file)
		{
			SG_SERROR( "error opening/reading file: \"%s\"",argv[1]);
			delete interface;
			return 1;
		}
		else
		{
			while(!feof(file) && intf->parse_line(intf->get_line(file, false)));
			fclose(file);
			delete interface;
			return 0;

		}
	}
}
#endif // HAVE_CMDLINE