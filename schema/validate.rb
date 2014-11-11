#!/usr/bin/env ruby

require 'json'
require 'json-schema'

def validate(schema, data, name)
  errors =  JSON::Validator.fully_validate(schema, data)
  if errors.empty?
    puts "#{name} passed validation!"
  else
    puts errors
    puts "#{name} failed validation."
    exit 1
  end
end

# Read in both the json meta-schema and the facter schema
JSON_META_SCHEMA = JSON.parse(File.read('schema/json-meta-schema.json'))
CFACTER_SCHEMA    = JSON.parse(File.read("schema/cfacter.json"))

# Validate that the cfacter schema itself is valid json
validate(JSON_META_SCHEMA, CFACTER_SCHEMA, "cfacter schema")

# Validate that the output facts match the cfacter schema
facts  = `bundle exec cfacter --json`
validate(CFACTER_SCHEMA, facts, "facts")
