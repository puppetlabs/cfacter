test_name "Running cfacter should not output anything to stderr"

on(hosts, cfacter) do |result|
  fail_test "Hostname fact is missing" unless stdout =~ /hostname\s*=>\s*\S*/
  fail_test "CFacter should not have written to stderr: #{stderr}" unless stderr == ""
end
