Pod::Spec.new do |s|
  s.name             = 'serialize'
  s.version          = '0.1.0'
  s.summary          = '一个简单快速的C++序列化实现'

  s.description      = '一个简单快速的C++序列化实现'

  s.homepage         = 'https://github.com/rexq57/serialize'
  s.license          = { :type => 'MIT', :file => 'LICENSE' }
  s.author           = { 'rexq57' => 'rexq57c@gmail.com' }
  s.source           = { :git => 'git@github.com:rexq57/serialize.git', :tag => s.version.to_s }

  s.ios.deployment_target = '7.0'
  s.macos.deployment_target = '10.8'


  s.header_dir = 'src/'
  s.header_mappings_dir = 'src/**/*'
  s.source_files = 'src/**/*{h,hpp,c,cc,cpp,mm,m}'

end
