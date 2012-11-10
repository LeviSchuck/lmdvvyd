
#pragma once

#include "macros.hpp"

{% for ns in config.namespaces %}namespace {{ns}} {
{% endfor %}
{% for ns in data.namespaces %}namespace {{ns}} {
{% endfor %}
class {{data.name}}_data;
class DLL_EXPORT {{data.name}} {
private:
  {{data.name}}_data * _data;
protected:

public:
  {{data.name}}();
  ~{{data.name}}();
};

{% for ns in data.rnamespaces %}
}// {{ns}}{% endfor %}
{% for ns in config.rnamespaces %}
}// {{ns}}{% endfor %}