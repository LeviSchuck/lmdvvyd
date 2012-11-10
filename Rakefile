require 'rubygems'
require 'rake'
require 'liquid'

SOURCE = "."
TAB = "    "
desc "Creates a class in the designated location"

task :mkclass do
	name = ENV["name"] || ""
	abort("No 'name' given!") unless name.length > 0
	location = ENV["loc"] || "."
	abort("No 'loc' given!") unless location.length > 0
	project = ENV["proj"] || ""
	abort("No 'proj' given!" ) unless project.length > 0
	masterproject = "lmdvvyd"
	
	namespace = ENV["ns"] || ""
	if namespace.length > 0
		namespace = "cordite::#{masterproject}::#{namespace}"
	else
		namespace = "cordite::#{masterproject}"
	end
	abort("Namespace 'ns' should NOT have whitespace!") unless ! namespace.match(/\s/)
	namespaces = namespace.split("::")

	
	if ! FileTest.directory?("#{project}/include/#{location}")
		puts "Creating directory include/#{location}"
		Dir.mkdir("#{project}/include/#{location}") 
	end
	if ! FileTest.directory?("#{project}/source/#{location}")
		puts "Creating directory source/#{location}"
		Dir.mkdir("#{project}/source/#{location}") 
	end
	
	if FileTest.exists?("#{project}/include/#{location}/#{name}.hpp")
		abort("You will end up overwriting include/#{location}/#{name}.hpp!")
	end
	if FileTest.exists?("#{project}/source/#{location}/#{name}.cpp")
		abort("You will end up overwriting source/#{location}/#{name}.cpp!")
	end

	open("#{project}/include/#{location}/#{name}.hpp", 'w') do |code|
		datapath = location or ""
		if datapath == "."
			datapath = ""
		end
		file = File.open("templates/header.hpp.tpl", "rb")
		contents = file.read
		code.puts Liquid::Template.parse(contents).render(
			'data' => {
				'name' => name,
				'cachedns' => namespace,
				'namespaces' => namespaces,
				'rnamespaces' => namespaces.reverse,
				'path' => datapath
			}
			)
	end
	open("#{project}/source/#{location}/#{name}.cpp", 'w') do |code|
		file = File.open("templates/body.cpp.tpl", "rb")
		contents = file.read
		code.puts Liquid::Template.parse(contents).render(
			'data' => {
				'name' => name,
				'cachedns' => namespace,
				'namespaces' => namespaces,
				'rnamespaces' => namespaces.reverse
			},
			
			)
	end

end