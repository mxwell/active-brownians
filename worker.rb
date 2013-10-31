#! /usr/bin/ruby
# encoding: utf-8

require "bunny"

if not File.exists? 'simulation' then
	puts "Where is 'simulation'?"
	exit 1
end

conn = Bunny.new(hostname: "algaysky",
	automatically_recover: false)
conn.start

ch = conn.create_channel
q = ch.queue("abp", durable: true)
qr = ch.queue("abp-results", durable: true)

puts " [*] Waiting for messages in #{q.name}. To exit press Ctrl+C"

begin
	q.subscribe(manual_ack: true, block: true) do |delivery_info, properties, body|
		puts " [x] Received message with.."
		mt = /(mu[^\}]*\})/m.match(body)
		if not mt
			puts "can't find mu. next."
			next
		end
		puts " " + mt.to_s
		File.open('by_rabbit.lua', 'w') do |file|
			file.puts body
		end
		cmd = "./simulation by_rabbit.lua 2>&1 | tee -a results/run.log"
		if not system(cmd) then
			puts "Error(s) during cmd execution"
			conn.close
			exit 1
		end
#		results_name = 'results/fixed.log'
#		if not File.exist? results_name then
#			puts "  can't found file with results"
#			puts "  not ack"
#			next
#		end
#		file = File.open(results_name, 'r')
#		results = file.reduce(:+)
#		file.close
#		qr.publish(results, persistent: true)
#		puts " [x] Results published to #{qr.name}"
		puts " [x] Done"
		ch.ack(delivery_info.delivery_tag)
	end
rescue Interrupt => _
	conn.close
end

