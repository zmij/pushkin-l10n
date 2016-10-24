/*
 * message_cereal.cpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <pushkin/l10n/message_cereal.hpp>
#include <pushkin/l10n/message_io.hpp>
#include <boost/lexical_cast.hpp>

namespace psst {
namespace l10n {

namespace {
void
read_l10n(message& msg, cereal::JSONInputArchive& ar, message::size_type& sz,
        message::optional_string const& domain)
{
    if (sz < 1) {
        throw cereal::Exception("Array size is too small for L10N");
    }
    ::std::string id;
    ar(id);
    msg.swap(message{ id, domain });
    --sz;
}
void
read_l10nn(message& msg, cereal::JSONInputArchive& ar, message::size_type& sz,
        message::optional_string const& domain)
{
    if (sz < 3) {
        throw cereal::Exception("Array size is too small for L10NN");
    }
    ::std::string single, plural;
    int n;
    ar(single, plural, n);
    msg.swap(message{ single, plural, n, domain });
    sz -= 3;
}
void
read_l10nc(message& msg, cereal::JSONInputArchive& ar, message::size_type& sz,
        message::optional_string const& domain)
{
    if (sz < 2) {
        throw cereal::Exception("Array size is too small for L10NC");
    }
    ::std::string context, id;
    ar(context, id);
    msg.swap(message{ context, id, domain });
    sz -= 2;
}
void
read_l10nnc(message& msg, cereal::JSONInputArchive& ar, message::size_type& sz,
        message::optional_string const& domain)
{
    if (sz < 4) {
        throw cereal::Exception("Array size is too small for L10NNC");
    }
    ::std::string context, single, plural;
    int n;
    ar(context, single, plural, n);
    msg.swap(message{ context, single, plural, n, domain });
    sz -= 4;
}

}  /* namespace  */

void
load(cereal::JSONInputArchive& ar, message& val)
{
    std::locale loc = ar.stream().getloc();
    message::optional_string domain;
    if (::std::has_facet< domain_name_facet >(loc)) {
        domain = std::use_facet< domain_name_facet >(loc).domain();
    }
    if (ar.nodeValue().IsArray()) {
        ar.startNode();
        message::size_type sz;
        ar( cereal::make_size_tag(sz) );
        std::string type_str;
        ar(type_str);
        --sz;
        message::message_type t = ::boost::lexical_cast< message::message_type >(type_str);

        switch (t) {
            case message::message_type::simple:
                read_l10n(val, ar, sz, domain);
                break;
            case message::message_type::plural:
                read_l10nn(val, ar, sz, domain);
                break;
            case message::message_type::context:
                read_l10nc(val, ar, sz, domain);
                break;
            case message::message_type::context_plural:
                read_l10nnc(val, ar, sz, domain);
                break;
            default: {
                std::ostringstream os;
                os << "Unexpected type for l10n::message: " << type_str;
                throw cereal::Exception( os.str() );
            }
        }
//        if (sz > 0) { TODO Read format args
//            read_format_args(ar, sz);
//        }
        ar.finishNode();
    } else if (ar.nodeValue().IsString()) {
        std::string str;
        ar(str);
        val.swap(message{str, domain});
    } else {
        val.swap(message{domain});
    }
}

void
save(cereal::JSONOutputArchive& ar, message const& msg)
{
    std::ostringstream os;
    os.imbue(ar.getloc());
    msg.write(os);
    ar(os.str());
}


}  /* namespace l10n */
}  /* namespace psst */
