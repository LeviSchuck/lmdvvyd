
{{config.includes}}
{{data.includes}}

{%if data.path %}
#include "{{data.path}}/{{data.name}}.hpp"
{% else %}
#include "{{data.name}}.hpp"
{% endif %}

{% for ns in config.namespaces %}namespace {{ns}} {
{% endfor %}
{% for ns in data.namespaces %}namespace {{ns}} {
{% endfor %}

class {{data.name}}_data
{
  public:
  {{data.name}}_data();
  ~{{data.name}}_data();
}

{{data.name}}::{{data.name}}()
{
  //Constructor
  _data = new {{data.name}}_data();
}

{{data.name}}::~{{data.name}}()
{
  //Destructor
  delete _data;
}


{{data.name}}_data::{{data.name}}_data()
{
  //Constructor
}

{{data.name}}_data::~{{data.name}}_data()
{
  //Destructor
}
{% for ns in data.rnamespaces %}
}// {{ns}}{% endfor %}
{% for ns in config.rnamespaces %}
}// {{ns}}{% endfor %}