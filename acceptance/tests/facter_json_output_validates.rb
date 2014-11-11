require 'json'
require 'json-schema'

test_name "Running cfacter --json should validate against the schema"

agents.each do |agent|
  step "Agent #{agent}: run 'cfacter --json' and validate"
  on(agent, cfacter('--json')) do
    # Validate that the output facts match the cfacter schema
    CFACTER_SCHEMA    = JSON.parse(File.read('../schema/cfacter.json'))
    fail_test "cfacter --json was invalid" unless JSON::Validator.validate!(CFACTER_SCHEMA, stdout)
  end
end
