#include "features/CombinedFeatures.h"
#include "lib/io.h"

class CCombinedFeatures;

CCombinedFeatures::CCombinedFeatures() : CFeatures(0l)
{
	feature_list=new CList<CFeatures*>(true);
}

CCombinedFeatures::CCombinedFeatures(const CCombinedFeatures & orig) : CFeatures(0l)
{
}

CFeatures* CCombinedFeatures::duplicate() const
{
	return new CCombinedFeatures(*this);
}

CCombinedFeatures::~CCombinedFeatures()
{
	delete feature_list;
}

void CCombinedFeatures::list_feature_objs()
{
	CFeatures* f;

	CIO::message(M_INFO, "BEGIN COMBINED FEATURES LIST - ");
	this->list_feature_obj();

	f=get_first_feature_obj();

	while (f)
	{
		f->list_feature_obj();
		f=get_next_feature_obj();
	}

	CIO::message(M_INFO, "END COMBINED FEATURES LIST - ");
}


