/*
 * message_cereal.cpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <pushkin/l10n/message_cereal.hpp>

namespace psst {
namespace l10n {

::std::locale::id domain_name_facet::id;

namespace {
void
read_l10n(cereal::JSONInputArchive& ar, size_type& sz)
{
    if (sz < 1) {
        throw cereal::Exception("Array size is too small for L10N");
    }
    std::string id;
    ar(id);
    swap(message{ id, domain_ });
    --sz;
}
void
read_l10nn(cereal::JSONInputArchive& ar, size_type& sz)
{
    if (sz < 3) {
        throw cereal::Exception("Array size is too small for L10NN");
    }
    std::string single, plural;
    int n;
    ar(single, plural, n);
    swap(message{ single, plural, n, domain_ });
    sz -= 3;
}
void
read_l10nc(cereal::JSONInputArchive& ar, size_type& sz)
{
    if (sz < 2) {
        throw cereal::Exception("Array size is too small for L10NC");
    }
    std::string context, id;
    ar(context, id);
    swap(message{ context, id, domain_ });
    sz -= 2;
}
void
read_l10nnc(cereal::JSONInputArchive& ar, size_type& sz)
{
    if (sz < 4) {
        throw cereal::Exception("Array size is too small for L10NNC");
    }
    std::string context, single, plural;
    int n;
    ar(context, single, plural, n);
    swap(message{ context, single, plural, n, domain_ });
    sz -= 4;
}

}  /* namespace  */

void
load(cereal::JSONInputArchive& ar, message& val)
{
    std::locale loc = ar.stream().getloc();
    if (std::has_facet< domain_name_facet >(loc)) {
        domain_ = std::use_facet< domain_name_facet >(loc).domain();
    }
    if (ar.nodeValue().IsArray()) {
        ar.startNode();
        size_type sz;
        ar( cereal::make_size_tag(sz) );
        std::string type_str;
        ar(type_str);
        --sz;
        type t = boost::lexical_cast< type >(type_str);

        switch (t) {
            case SIMPLE:
                read_l10n(ar, sz);
                break;
            case PLURAL:
                read_l10nn(ar, sz);
                break;
            case CONTEXT:
                read_l10nc(ar, sz);
                break;
            case CONTEXT_PLURAL:
                read_l10nnc(ar, sz);
                break;
            default: {
                std::ostringstream os;
                os << "Unexpected type for l10n::message: " << type_str;
                throw cereal::Exception( os.str() );
            }
        }
        if (sz > 0) {
            read_format_args(ar, sz);
        }
        ar.finishNode();
    } else if (ar.nodeValue().IsString()) {
        std::string str;
        ar(str);
        swap(message{str, domain_});
    } else {
        swap(message{domain_});
    }
}

void
save(cereal::JSONOutputArchive&, message const&)
{
    std::ostringstream os;
    os.imbue(ar.getloc());
    write(os);
    ar(os.str());
}


}  /* namespace l10n */
}  /* namespace psst */
